# Maintaining this fork of `@op-engineering/op-sqlite`

This is Vendora's fork of [`OP-Engineering/op-sqlite`](https://github.com/OP-Engineering/op-sqlite).
We keep it **as close to upstream as possible**, carrying only a small, replayable set of patches on top
of an unmodified upstream release.

---

## TL;DR

- **Why this fork exists:** two patches upstream doesn't have —
  1. **React Native Windows support** (`windows/**` native project + wiring), and
  2. an **Android `sqlite-vec` libm fix** (preload `libm` so the prebuilt extension loads on 32-bit ARM).
- **Our build = `<upstream release>` + the two patches above.** Nothing else is changed.
- **Version ceiling:** `@powersync/op-sqlite` peer-depends on `@op-engineering/op-sqlite@^13 || ^14 || ^15`.
  **Do not move past the `15.2.x` line** until PowerSync ships a release that supports op-sqlite `16.x`.
  (See [Compatibility](#compatibility) — this is the most important rule in this file.)
- **Pinning:** the Vendora monorepo pins a **named tag** like `#v15.2.14-vendora.1`, never a raw commit SHA.
- **Upgrading** = replay our two patches onto the new upstream tag, rebuild `lib/`, re-tag, re-pin.

---

## Compatibility

> ⚠️ **The op-sqlite version is capped by PowerSync, not by upstream's latest.**

`@powersync/op-sqlite` declares:

```jsonc
"peerDependencies": { "@op-engineering/op-sqlite": "^13.0.0 || ^14.0.0 || ^15.0.0" }
```

So the **latest version we may use is the highest `15.2.x` release** (currently `15.2.14`). Upstream `16.x`
exists but is **out of bounds** until a future `@powersync/op-sqlite` widens its peer range to include it.

**Before every upgrade, check the ceiling:**

```bash
npm view @powersync/op-sqlite peerDependencies        # what op-sqlite range does PowerSync allow?
npm view @op-engineering/op-sqlite versions --json     # what upstream releases exist?
```

Pick the **highest upstream release that satisfies PowerSync's range**. If PowerSync ever adds `^16.0.0`,
only then is `16.x` a candidate (and re-test the PowerSync adapter against it).

---

## The mental model

Upstream ships releases (`15.2.12`, `15.2.13`, `15.2.14`, …). We take one **unchanged** and stack our two
patches on top:

```
upstream 15.2.14  ──●
                     ╲
   our patches        ●  feat(windows): React Native Windows support
                       ●  fix(android): preload libm for sqlite-vec
                        = tag v15.2.14-vendora.1   (+ built lib/, see below)
```

The discipline: **upstream history stays pristine underneath; our patches sit cleanly on top**, so they can be
**replayed onto the next upstream release** (a rebase). We pin a **tag that names the base** so the version is
legible — `v15.2.14-vendora.1` reads as "upstream 15.2.14, our patch revision 1." `git diff 15.2.14 v15.2.14-vendora.1`
shows exactly our delta.

(The package's in-repo `version` field is always `0.0.0` — upstream stamps the real number only at npm-publish
time. We install from GitHub, so **the git tag is our version of record**.)

---

## Branch & tag model

- **`vendora-patches`** — the long-lived branch. Holds our **source patches + this doc**, and nothing generated.
  We rebase it forward onto each new upstream release.
- **`v<upstream>-vendora.<n>` tags** — immutable releases = `vendora-patches` **plus one commit containing the
  built `lib/`** (see below). The monorepo pins these.
- The fork's `main` is an old upstream mirror; it is **not** the source of truth. `vendora-patches` + tags are.

One-time git setup in a fresh clone:

```bash
git remote add upstream https://github.com/OP-Engineering/op-sqlite.git
git fetch upstream --tags
```

> Note: `gh repo clone` of a fork auto-adds `upstream` and fetches it, so origin's and upstream's tags share one
> namespace. When in doubt, resolve a base by its **divergence point**, e.g.
> `git merge-base vendora-patches <upstream-tag>`, rather than trusting a bare tag name.

---

## About the committed `lib/`

`lib/` is op-sqlite's **build output** — `lib/module/*.js` (compiled from `src/*.ts`) and `lib/typescript/**/*.d.ts`.
Upstream **gitignores it** and ships it only in the npm tarball. We install from **GitHub**, which has no build
step on install (bun doesn't run the package's `prepare`), and `package.json` resolves consumers into `lib/`
(`"types": "./lib/typescript/src/index.d.ts"`, `"main"/"exports".default → ./lib/module/index.js`). So we commit a
prebuilt `lib/` **on the release tag** so the package works when installed from Git.

**`lib/` is generated, never hand-edited.** It is a pure function of `src/` + `babel.config.js` + `tsconfig.build.json`.
Whenever those change, regenerate it:

```bash
yarn install            # Yarn Berry (yarn@4); installs bob + typescript
yarn bob build          # emits lib/module + lib/typescript  (op-sqlite's build step)
git add -f lib          # -f because lib/ is gitignored
```

> Shortcut that's only valid when the inputs are unchanged: if a bump changes nothing under `src/` (nor the two
> tsconfigs / babel config) — which is common for upstream **patch** releases — the previous tag's `lib/` is already a
> faithful build and may be carried forward. **Verify** before relying on it:
> `git diff <prev-tag> vendora-patches -- src babel.config.js tsconfig.build.json tsconfig.json` must be empty.
> If in any doubt, just rebuild.

---

## Upgrade playbook (worked example: `15.2.14 → <next 15.2.x>`)

```bash
# 0. Confirm the ceiling (see Compatibility). Pick the highest 15.2.x PowerSync still allows.
NEW=15.2.15            # example
OLD=15.2.14            # the base named in our current tag

# 1. Get upstream tags
git fetch upstream --tags

# 2. Replay our two patches onto the new release
git checkout vendora-patches
git rebase --onto "$NEW" "$OLD" vendora-patches
#   Conflicts are rare: our patches are almost all net-new windows/** files plus a few lines in
#   cpp/bridge.cpp, src/*.ts and package.json. Resolve, keeping upstream's changes + re-applying ours.

# 3. Regenerate lib/ (skip only if the verify-diff above is empty)
yarn install && yarn bob build

# 4. Smoke-test: build the example (or POS) on Windows + iOS + Android.
#    Specifically exercise sqlite-vec on a 32-bit ARM (armeabi-v7a) Android build — that's what the libm fix targets.

# 5. Cut the release tag (vendora-patches + freshly built lib/)
git checkout -b "release-v${NEW}-vendora.1" vendora-patches
git add -f lib
git commit -m "build: lib/ for v${NEW}-vendora.1"
git tag "v${NEW}-vendora.1"
git push origin vendora-patches --force-with-lease
git push origin "v${NEW}-vendora.1"

# 6. Re-pin the monorepo (next section), bun install, /changeset, commit.
```

**Versioning the tag:** new upstream base → reset the suffix (`v15.2.15-vendora.1`). Same base but you changed
*our* patches → bump the suffix (`v15.2.14-vendora.2`).

---

## Pinning in the Vendora monorepo

Update **both** `resolutions` and `overrides` in the root `package.json` (bun honors `overrides`; keep them in sync):

```jsonc
"@op-engineering/op-sqlite": "github:Virtual-Fulfillment-Technologies-Inc/op-sqlite#v15.2.14-vendora.1"
```

Also bump the nominal version in `apps/pos/package.json` (`"@op-engineering/op-sqlite": "15.2.14"`) so the file isn't
misleading — it's overridden either way. Then `bun install`, commit `bun.lock`, and add a patch `/changeset`.

**Rollback** is trivial: re-pin to the previous `v*-vendora.*` tag and `bun install`. Tags are immutable, so the old
build is always available.

---

## The two patches, in detail

### 1. `feat(windows): add React Native Windows support`
Adds the `windows/**` RNW native project, the `react-native-windows` dependency + codegen config in `package.json`,
`NuGet.config`, and small `cpp/`/`src/` additions (Windows path constants `WINDOWS_LOCAL_FOLDER` etc., MSVC-safe
pointer inits). Upstream has no Windows target; this is the bulk of the fork.

### 2. `fix(android): preload libm so sqlite-vec resolves ceil on armeabi-v7a`
The prebuilt `libsqlite_vec.so` references `ceil`/`floor` (libm) but isn't linked against libm. On 32-bit ARM the
dynamic linker can't resolve those symbols, so `sqlite3_load_extension` aborts startup with
`cannot locate symbol "ceil"`. The fix preloads libm into the global symbol scope **before** loading the extension,
in `cpp/bridge.cpp`:

```cpp
#if defined(__ANDROID__)
  dlopen("libm.so", RTLD_NOW | RTLD_GLOBAL);
#endif
```

Android-only, gated on `OP_SQLITE_USE_SQLITE_VEC`; no effect on iOS/Windows. (Originated as Vendora PR #1720, which
used a `bun patch`; it lives here in the fork instead so it survives version bumps and applies to the Git-pinned dep.)

---

## If maintaining this ever gets heavy

Two escape hatches, in increasing order of effort:
1. **Publish a private package** (`@vft/op-sqlite@<upstream>-windows.N`) to GitHub Packages via a tiny CI job that
   runs `bob build` and publishes on tag. Consumers pin a real semver; no committed `lib/`.
2. **Upstream the Windows support** to OP-Engineering. If accepted, the Windows patch disappears and we track npm
   releases directly (the libm fix may also be upstreamable as a build-time `-lm` link or a load-time preload).
