type StorageOptions = {
    location?: string;
    encryptionKey?: string;
};
export declare class Storage {
    private dbPromise;
    constructor(options: StorageOptions);
    private getDb;
    getItem(key: string): Promise<string | undefined>;
    getItemSync(_key: string): string | undefined;
    setItem(key: string, value: string): Promise<void>;
    setItemSync(_key: string, _value: string): void;
    removeItem(key: string): Promise<void>;
    removeItemSync(_key: string): void;
    clear(): Promise<void>;
    clearSync(): void;
    getAllKeys(): Promise<string[]>;
    delete(): void;
}
export {};
//# sourceMappingURL=Storage.web.d.ts.map