# Config
The config subsystem is a generic configuration manager that handles the storage and retrieval of vairables from an on-board EEPROM, model m95256.
The `config` struct contains all the needed data:
```c
struct config {
    uint32_t version;
    uint16_t address;
    bool dirty;
    size_t size;
    void *data;
};
```
`version` stores a version string that is used during to verify that the read data is right. By changing the version string the data gets automatically overwritten with default values.

The `config` struct is rendered opaque by the use of the `config_t` type. This avoids direct access to `config`'s content.
