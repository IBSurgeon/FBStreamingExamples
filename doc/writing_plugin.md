# Writing your own plugin for the fb_streaming service

You can write your own plugin for publishing events based on replication log analysis for the `fb_streaming` service.

The plugin can be written in C++ or Delphi.

To create a plugin, you need to write an implementation of three interfaces:

* `IStreamPlugin` - a handler for non-transactional events and a transaction start event;
* `IStreamedTransaction` - a handler for transactional events;
* `IStreamPluginFactory` - a factory for creating an instance of a plugin with the `IStreamPlugin` interface.

To simplify the implementation of these interfaces, there are special template classes with the `Impl` suffix: `IStreamPluginImpl`, `IStreamedTransactionImpl` and `IStreamPluginFactoryImpl`. The first parameter of the template is the name of the implementation class, and the second is the name of the wrapper class for the status. There are two classes available as status wrappers: `ThrowStatusWrapper` and `CheckStatusWrapper`.

Let's say we decide to create classes that implement these interfaces with the following names: `MyStreamPlugin`, `MyStreamedTransaction` and `MyStreamPluginFactory`. Then the definition of these classes should look like this:

```cpp
namespace MyPlugin {

    class MyStreamedTransaction;

    class MyStreamPlugin : public Firebird::IStreamPluginImpl<MyStreamPlugin, Firebird::ThrowStatusWrapper>
    {
    public:
        MyStreamPlugin() = delete;
        MyStreamPlugin(Firebird::IMaster* master, Firebird::IConfig* config,
            Firebird::IStringEncodeUtils* encodeUtils, Firebird::IStreamLogger* logger);
        virtual ~MyStreamPlugin();

        // IReferenceCounted implementation
        void addRef() override;
        int release() override;

        // IPluginBase implementation
        void setOwner(Firebird::IReferenceCounted* o) override;
        Firebird::IReferenceCounted* getOwner() override;

        // IStreamPlugin implementation
        FB_BOOLEAN init(Firebird::ThrowStatusWrapper* status, Firebird::IAttachment* attachment) override;
        void finish(Firebird::ThrowStatusWrapper* status) override;
        void startSegment(Firebird::ThrowStatusWrapper* status, SegmentHeaderInfo* segmentHeader) override;
        void finishSegment(Firebird::ThrowStatusWrapper* status) override;
        void startBlock(ThrowStatusWrapper* status, ISC_UINT64 blockOffset, unsigned blockLength) override;
        void setSegmentOffset(ISC_UINT64 offset) override;
        Firebird::IStreamedTransaction* startTransaction(Firebird::ThrowStatusWrapper* status, ISC_INT64 number) override;
        void setSequence(Firebird::ThrowStatusWrapper* status, const char* name, ISC_INT64 value) override;
        FB_BOOLEAN matchTable(Firebird::ThrowStatusWrapper* status, const char* relationName) override;
        Firebird::IStreamedTransaction* getTransaction(Firebird::ThrowStatusWrapper* status, ISC_INT64 number) override;
        void cleanupTransaction(Firebird::ThrowStatusWrapper* status, ISC_INT64 number) override;
        void cleanupTransactions(Firebird::ThrowStatusWrapper* status) override;
        void log(unsigned level, const char* message) override;
    private:
        // ...
    };

    class MyStreamedTransaction : public Firebird::IStreamedTransactionImpl<MyStreamedTransaction, Firebird::ThrowStatusWrapper>
    {
    public:
        MyStreamedTransaction() = delete;
        MyStreamedTransaction(MyStreamPlugin* streamPlugin, ISC_INT64 number);
        virtual ~MyStreamedTransaction();

        // IDisposable implementation
        void dispose() override;

        // IStreamedTransaction implementation
        void prepare(Firebird::ThrowStatusWrapper* status) override;
        void commit(Firebird::ThrowStatusWrapper* status) override;
        void rollback(Firebird::ThrowStatusWrapper* status) override;
        void startSavepoint(Firebird::ThrowStatusWrapper* status) override;
        void releaseSavepoint(Firebird::ThrowStatusWrapper* status) override;
        void rollbackSavepoint(Firebird::ThrowStatusWrapper* status) override;
        void insertRecord(Firebird::ThrowStatusWrapper* status, const char* name, Firebird::IStreamedRecord* record) override;
        void updateRecord(Firebird::ThrowStatusWrapper* status, const char* name, Firebird::IStreamedRecord* orgRecord,
            Firebird::IStreamedRecord* newRecord) override;
        void deleteRecord(Firebird::ThrowStatusWrapper* status, const char* name, Firebird::IStreamedRecord* record) override;
        void executeSql(Firebird::ThrowStatusWrapper* status, const char* sql) override;
        void executeSqlIntl(Firebird::ThrowStatusWrapper* status, unsigned charset, const char* sql) override;
        void storeBlob(Firebird::ThrowStatusWrapper* status, ISC_QUAD* blob_id,
            ISC_INT64 length, const unsigned char* data) override;
    private:
        MyStreamPlugin* m_streamPlugin;
        ISC_INT64 m_number;
        // other fields
    };


    class MyStreamPluginFactory : public Firebird::IStreamPluginFactoryImpl<MyStreamPluginFactory, Firebird::ThrowStatusWrapper>
    {
    private:
        Firebird::IMaster* m_master = nullptr;
    public:
        MyStreamPluginFactory() = delete;
        explicit MyStreamPluginFactory(Firebird::IMaster* master);

    Firebird::IStreamPlugin* createPlugin(Firebird::ThrowStatusWrapper* status, Firebird::IConfig* config,
        Firebird::IStringEncodeUtils* encodeUtils, Firebird::IStreamLogger* logger) override;
    };

    // ...
} // namespace MyPlugin
```

After you have written the implementation of these interfaces, you need to register the plugin at the entry point. For plugins of the `fb_streaming` service, the entry point is a function named `fb_stream_plugin`.

An example of registering a plugin is given in the following code:

```cpp

#include "../../include/StreamingInterface.h"

using namespace Firebird;

extern "C" {

    FB_DLL_EXPORT void fb_stream_plugin(IMaster* master, IStreamPluginManager* pm)
    {
        // creating a factory to register a plugin
        auto factory = new MyPlugin::MyStreamPluginFactory(master);
        // registering a plugin named "my_plugin"
        pm->registerPluginFactory("my_plugin", factory);
    }
} // extern C
```

A complete example of creating your own plugin for the `fb_streaming` service with source code can be found at <https://github.com/IBSurgeon/FBStreamingExamples>

The following is a description of the interfaces you will need when implementing your own plugins.

---

## Interface `IStreamPlugin`

Interface `IStreamPlugin` - contains functions for initializing and terminating the plugin, filtering tables by name, handlers for starting and ending the processing of a replication log segment, starting a transaction, setting the generator (sequence) value, and other functions.

Interface `IStreamPlugin` is a descendant of interface `IPluginBase`. This means that in addition to the native functions of the interface, you must implement the methods: `setOwner` and `getOwner`. In addition, you must implement the methods of the reference counting interface `IReferenceCounted`: `addRef` and `release`.

### `init` function

```cpp
FB_BOOLEAN init(Status* status, IAttachment* attachment)
```

The `init` function is used to initialize the plugin. The `attachment` parameter is passed an instance of the master database connection. This may be necessary to read metadata required for the plugin to function. The function returns `true` if successful, and `false` otherwise.

> [!NOTE]
> The `init` function may be called more than once, since the database connection is broken if idle.

### Function `finish`

```cpp
void finish(Status* status)
```

The `finish` function is designed to clean up and free structures allocated during the plugin initialization process.

### `startSegment` function

```cpp
void startSegment(Status* status, SegmentHeaderInfo* segmentHeader)
```

The `startSegment` function is designed to handle the event of starting the processing of a replication segment.
The `segmentHeader` parameter is passed a structure with information from the replication segment header.

The `SegmentHeaderInfo` structure looks like this:

```cpp
struct SegmentHeaderInfo
{
char name[256];
char guid[39];
ISC_UINT64 sequence;
ISC_UINT64 length;
ISC_USHORT version;
ISC_USHORT state;
};
```

Description of structure fields:

* `name` - replication segment file name;
* `guid` - master database GUID;
* `sequence` - replication segment number;
* `length` - replication segment length in bytes;
* `version` - replication protocol version;
* `state` - replication segment state. 0 - free, 1 - used, 2 - full, 3 - archive.

### `finishSegment` function

```cpp
void finishSegment(Status* status)
```

The `finishSegment` function is designed to handle the event of completion of replication segment processing.

### `startBlock` function

```cpp
void startBlock(ThrowStatusWrapper* status, ISC_UINT64 blockOffset, unsigned blockLength) override;
```

The `startBlock` function is designed to handle the event of the start of a block in the replication segment file. Usually, events are not sent to the file one by one, but in whole blocks. If the transaction is short enough, the start and end of the block coincide with the start and end of the transaction. In your plugins, you will most likely make this method a stub. It is used in the `fb_repl_print` application to print information about the start of a block.

### `setSegmentOffset` function

```cpp
void setSegmentOffset(ISC_UINT64 offset) override;
```

Saves the offset (in bytes) from the replication segment file. In most plugins, this method is simply a stub. However, it is used in the `fb_repl_print` application to specify the offset for each event when printing.

### `startTransaction` function

```cpp
IStreamedTransaction* startTransaction(Status* status, ISC_INT64 number)
```

The `startTransaction` function is designed to handle the transaction start event. The `number` parameter is passed the transaction number. The function must create an instance of the class implementing the `IStreamedTransaction` interface and return a pointer to it.

The list of pointers to the `IStreamedTransaction` interface must be stored in the internal structures of the class implementing the `IStreamPlugin` interface. This is necessary because the completion of the transaction completion event can be postponed indefinitely (the transaction does not necessarily complete in the same segment in which it was started).

### Function `setSequence`

```cpp
void setSequence(Status* status, const char* name, ISC_INT64 value)
```

The `setSequence` function is designed to handle the event of setting the value of the generator (sequence). The name of the generator (sequence) is passed to the `name` parameter, and the value is passed to the `value` parameter.

### Function `matchTable`

```cpp
FB_BOOLEAN matchTable(Status* status, const char* relationName)
```

The `matchTable` function is designed to filter tables by name. If the function returns `true`, then the `INSERT`, `UPDATE` and `DELETE` events will be processed for the table named `relationName`, otherwise the table events will be ignored.

### `getTransaction` function

```cpp
IStreamedTransaction* getTransaction(Status* status, ISC_INT64 number)
```

The `getTransaction` function returns an instance of the class implementing the `IStreamedTransaction` interface by the transaction number specified in the `number` parameter.

### `cleanupTransaction` function

```cpp
void cleanupTransaction(Status* status, ISC_INT64 number)
```

The `cleanupTransaction` function removes an instance of the class implementing the `IStreamedTransaction` interface from the internal list by the transaction number specified in the `number` parameter.

### Function `cleanupTransactions`

```cpp
void cleanupTransactions(Status* status)
```

The `cleanupTransactions` function is designed to handle the event of cleaning active (not completed) transactions (instances of the `IStreamedTransaction` interface classes). For each active transaction in the transaction list, the `rollback` function is called and the transaction is removed from the internal list.

### `log` function

```cpp
void log(unsigned level, const char* message)
```

The `log` function is used to write a message to the `fb_streaming` service log. This function is convenient for debugging the plugin.

The `level` parameter is used to pass the logging level. Logging levels are described in the constants of the `IStreamLogger` interface:

* `IStreamLogger::LEVEL_TRACE`
* `IStreamLogger::LEVEL_DEBUG`
* `IStreamLogger::LEVEL_INFO`
* `IStreamLogger::LEVEL_WARN`
* `IStreamLogger::LEVEL_ERROR`
* `IStreamLogger::LEVEL_CRITICAL`

The `message` parameter is used to pass the message text.

The minimum level of message logging is configured in the `fb_streaming.conf` configuration file in the `logLevel` parameter.

---

## Interface `IStreamedTransaction`

Interface `IStreamedTransaction` - contains functions for registering events in the context of a transaction.

Interface `IStreamedTransaction` is a descendant of interface `IDisposable`. This means that in addition to the native interface functions, you must implement the `dispose` method, which is designed to free allocated resources and destroy the class instance.

### Function `prepare`

```cpp
void prepare(Status* status)
```

The `prepare` function is designed to handle the event of execution of the first phase of the two-phase transaction confirmation.

### Function `commit`

```cpp
void commit(Status* status)
```

The `commit` function is designed to handle the transaction confirmation event.

### `rollback` function

```cpp
void rollback(Status* status)
```

The `rollback` function is designed to handle the transaction rollback event.

### `startSavepoint` function

```cpp
void startSavepoint(Status* status)
```

The `startSavepoint` function is designed to handle the savepoint installation event.

### `releaseSavepoint` function

```cpp
void releaseSavepoint(Status* status)
```

The `releaseSavepoint` function is designed to handle the savepoint release event.

### `rollbackSavepoint` function

```cpp
void rollbackSavepoint(Status* status)
```

The `rollbackSavepoint` function is designed to handle the savepoint rollback event.

### Function `insertRecord`

```cpp
void insertRecord(Status* status, const char* name, IStreamedRecord* record)
```

The `insertRecord` function is designed to handle the event of inserting a new record into a table. The `name` parameter is passed to the table name. The `record` parameter is passed to the `IStreamedRecord` interface pointer, designed to access the field values of the inserted record.

### Function `updateRecord`

```cpp
void updateRecord(Status* status, const char* name, IStreamedRecord* orgRecord, IStreamedRecord* newRecord)
```

The `updateRecord` function is designed to handle the event of updating a record in a table. The `name` parameter is passed to the table name. The `orgRecord` parameter is passed a pointer to the `IStreamedRecord` interface, intended for accessing the old field values of the record being updated. The `newRecord` parameter is passed a pointer to the `IStreamedRecord` interface, intended for accessing the new field values of the record being updated.

### `deleteRecord` function

```cpp
void deleteRecord(Status* status, const char* name, IStreamedRecord* record)
```

The `deleteRecord` function is intended to handle the event of deleting a record from a table. The `name` parameter is passed the table name. The `record` parameter is passed a pointer to the `IStreamedRecord` interface, intended for accessing the field values of the record being deleted.

### Function `executeSql`

```cpp
void executeSql(Status* status, const char* sql)
```

The `executeSql` function is designed to handle the event of executing a SQL query (usually these are DDL queries for modifying database metadata). The text of the SQL query is passed to the `sql` parameter. The SQL query will be in ANSI or UTF8 encoding.

### Function `executeSqlIntl`

```cpp
void executeSqlIntl(Status* status, unsigned charset, const char* sql)
```

The `executeSqlIntl` function is designed to handle the event of executing a SQL query (usually these are DDL queries for modifying database metadata). Unlike the previous function, this one takes into account the encoding (parameter `charset`) in which the query was passed. The text of the SQL query is passed to the `sql` parameter.

### Function `storeBlob`

```cpp
void storeBlob(Status* status, ISC_QUAD* blob_id,
ISC_INT64 length, const unsigned char* data)
```

The `storeBlob` function is designed to handle the event of saving BLOB data. The `blob_id` parameter is passed the BLOB identifier, `length` is the length of the data in bytes, `data` is a pointer to the data.

---

## Interface `IStreamPluginFactory`

Interface `IStreamPluginFactory` - contains functions for creating an instance of the `IStreamPlugin` interface class.

### Function `createPlugin`

```cpp
IStreamPlugin* createPlugin(Status* status, IConfig* config, IStringEncodeUtils* encodeUtils, IStreamLogger* logger)
```

The `createPlugin` function is designed to create an instance of the `IStreamPlugin` interface class. The `createPlugin` function returns a pointer to the newly created instance. The `config` parameter is passed a pointer to the interface for reading the plugin configuration. The `logger` parameter is passed a pointer to the logging interface of the `fb_streaming` service.

The `encodeUtils` parameter is passed a pointer to the service interface that helps to recode strings into different encodings. The point is that in replication segments, table fields with character data can be stored in different encodings (as the user defined them in the table). When writing a plugin, you usually need to operate with a string in one specific encoding, for example, UTF-8. Of course, you can implement string conversion yourself, but since the `fb_streaming` service loads the `icu` library anyway, we decided to provide a special interface that simplifies this task.

> [!WARNING]
> Before returning a pointer to an instance of the `IStreamPlugin` interface class, do not forget to increase the reference counter by calling `addRef()`.

---

## `IStreamedRecord` Interface

The `IStreamedRecord` interface is used to access field values of new or old records.

### `getCount` Function

```cpp
unsigned getCount()
```

The `getCount` function returns the number of fields in a record.

### `getField` Function

```cpp
IStreamedField* getField(unsigned index)
```

The `getField` function returns a pointer to the `IStreamedField` interface, which is used to access data for a specific field, and can also be used to obtain information about the field type, name, size, and other attributes. The `index` parameter is passed the field index in the record.

### `getRawLength` function

```cpp
unsigned getRawLength()
```

`getRawLength` function returns the record size in bytes.

### `getRawData` function

```cpp
const unsigned char* getRawData()
```

`getRawData` function returns a pointer to the raw data of the record.

---

## `IStreamedField` Interface

The `IStreamedField` interface is designed to access data for a specific field. It can also be used to obtain information about the field type, its name, dimensions, and other attributes.

### `getName` Function

```cpp
const char* getName()
```

The `getName` function returns the field name.

### `getType` Function

```cpp
unsigned getType()
```

The `getType` function returns the field type.

### `getSubType` Function

```cpp
int getSubType()
```

The `getSubType` function returns the field subtype (for `BLOB` fields).

### `getScale` function

```cpp
int getScale()
```

The `getScale` function returns the scale of the field (for `NUMERIC` and `DECIMAL` fields).

### `getLength` function

```cpp
unsigned getLength()
```

The `getLength` function returns the size of the data buffer allocated for the field value.

### `getCharSet` function

```cpp
unsigned getCharSet()
```

The `getCharSet` function returns the field encoding (for `CHAR`, `VARCHAR`, and `BLOB` fields).

### `getData` function

```cpp
const void* getData()
```

The `getData` function returns a pointer to the field value. If the field is `NULL`, then `nullptr` is returned.

### `isKey` function

```cpp
FB_BOOLEAN isKey()
```

The `isKey` function returns whether the given field is included in the primary key of the table.

### Function `keyPosition`

```cpp
unsigned keyPosition()
```

The function `keyPosition` returns the position of the field in the primary key, if it is included in the primary key, otherwise it returns 0.

---

## `IStreamLogger` interface

The `IStreamLogger` interface is designed to write messages to the `fb_streaming` service log. This interface is convenient for debugging the plugin.

Logging levels are described in the constants of the `IStreamLogger` interface:

* `IStreamLogger::LEVEL_TRACE`
* `IStreamLogger::LEVEL_DEBUG`
* `IStreamLogger::LEVEL_INFO`
* `IStreamLogger::LEVEL_WARN`
* `IStreamLogger::LEVEL_ERROR`
* `IStreamLogger::LEVEL_CRITICAL`

The minimum message logging level is configured in the `fb_streaming.conf` configuration file in the `logLevel` parameter.

### `getLevel` function

```cpp
unsigned getLevel()
```

The `getLevel` function returns the logging level.

### `log` function

```cpp
void log(unsigned level, const char* message)
```

The `log` function is used to write a message to the `fb_streaming` service log. This function is convenient for debugging the plugin.

The `level` parameter is used to pass the logging level.

The `message` parameter is used to pass the message text.

### Function `trace`

```cpp
void trace(const char* message)
```

The `trace` function is designed to write a message to the `fb_streaming` service log with the `IStreamLogger::LEVEL_TRACE` logging level. The message text is passed to the `message` parameter.

### Function `debug`

```cpp
void debug(const char* message)
```

The `debug` function is designed to write a message to the `fb_streaming` service log with the `IStreamLogger::LEVEL_DEBUG` logging level. The message text is passed to the `message` parameter.

### Function `info`

```cpp
void info(const char* message)
```

The `info` function is used to write a message to the `fb_streaming` service log with the `IStreamLogger::LEVEL_INFO` logging level. The message text is passed to the `message` parameter.

### Function `warning`

```cpp
void warning(const char* message)
```

The `warning` function is used to write a message to the `fb_streaming` service log with the `IStreamLogger::LEVEL_WARN` logging level. The message text is passed to the `message` parameter.

### Function `error`

```cpp
void error(const char* message)
```

The `error` function is designed to write a message to the `fb_streaming` service log with the logging level `IStreamLogger::LEVEL_ERROR`. The message text is passed to the `message` parameter.

### Function `critical`

```cpp
void critical(const char* message)
```

The `critical` function is designed to write a message to the `fb_streaming` service log with the logging level `IStreamLogger::LEVEL_CRITICAL`. The message text is passed to the `message` parameter.

---

## Interface `IStringEncodeUtils`

Interface `IStringEncodeUtils` provides functions for converting strings from one encoding to another. To simplify working with this interface, we wrote a high-level wrapper [`StringEncodeHelper`](../src/encoding/StringEncodeHelper.h).

### Function `getConverterById`

```cpp
IStringConverter* getConverterById(Status* status, unsigned charsetId)
```

Returns an instance of the `IStringConverter` interface, which is designed to convert strings from their encoding to one of the unicode encodings. The `charsetId` parameter specifies the Firebird encoding identifier for the `IStringConverter` converter. Firebird encoding identifiers are listed in the [`charsets.h`](../src/common/charsets.h) file.

### `getConverterByName` function

```cpp
IStringConverter* getConverterByName(Status* status, const char* charsetName)
```

Returns an instance of the `IStringConverter` interface, which is designed to convert strings from their encoding to one of the unicode encodings. The `charsetName` parameter specifies the name of the Firebird character set for the `IStringConverter` converter.

### `convertCharset` function

```cpp
ISC_UINT64 convertCharset(Status* status, IStringConverter* srcConverter, IStringConverter* dstConverter,
const char* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize)
```

Converts a string from one encoding to another using instances of the `IStringConverter` interface of converters for the source and target encodings. Converter instances can be obtained using the `getConverterById` or `getConverterByName` functions.

Parameters of the `convertCharset` function:

* `status` - status vector;
* `srcConverter` - converter instance for the source encoding;
* `dstConverter` - an instance of the converter for the target encoding;
* `src` - the source string;
* `srcSize` - the size of the source string;
* `destBuffer` - a buffer to place the target string;
* `destBufferSize` - the size of the buffer for the target string.

The function returns the length of the target string if the conversion was successful.

### Function `convertUtf8ToWCS`

```cpp
ISC_UINT64 convertUtf8ToWCS(Status* status, const char* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize)
```

Converts a string from the `utf-8` encoding to a wide character string (of type `wchar_t`).

Function parameters:

* `status` - status vector;
* `src` - source string;
* `srcSize` - size of the source string;
* `destBuffer` - buffer for placing the target string;
* `destBufferSize` - size of the buffer for the target string.

The function returns the length of the target string if the conversion was successful.

> [!NOTE]
> **Note**: `wchar_t` is not a platform independent type, its size depends on the operating system on which it is compiled.

### Function `convertUtf8ToUtf16`

```cpp
ISC_UINT64 convertUtf8ToUtf16(Status* status, const char* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize)
```

Converts a string from `utf-8` encoding to a string with `utf-16` encoding.

Function parameters:

* `status` - status vector;
* `src` - source string;
* `srcSize` - size of source string;
* `destBuffer` - buffer for placing the target string;
* `destBufferSize` - size of the buffer for the target string.

The function returns the length of the target string if the conversion was successful.

### Function `convertUtf8ToUtf32`

```cpp
ISC_UINT64 convertUtf8ToUtf32(Status* status, const char* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize)
```

Converts a string from `utf-8` encoding to a string with `utf-32` encoding.

Function parameters:

* `status` - status vector;
* `src` - source string;
* `srcSize` - size of source string;
* `destBuffer` - buffer for placing the target string;
* `destBufferSize` - size of the buffer for the target string.

The function returns the length of the target string if the conversion was successful.

### Function `convertUtf16ToWCS`

```cpp
ISC_UINT64 convertUtf16ToWCS(Status* status, const void* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize)
```

Converts a string from the `utf-16` encoding to a wide character string (of type `wchar_t`).

Function parameters:

* `status` - status vector;
* `src` - source string;
* `srcSize` - size of the source string;
* `destBuffer` - buffer for placing the target string;
* `destBufferSize` - size of the buffer for the target string.

The function returns the length of the target string if the conversion was successful.

> [!NOTE]
> **Note**: `wchar_t` is not a platform independent type, its size depends on the operating system on which it is compiled.

### Function `convertUtf16ToUtf8`

```cpp
ISC_UINT64 convertUtf16ToUtf8(Status* status, const void* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize)
```

Converts a string from `utf-16` encoding to a string with `utf-8` encoding.

Function parameters:

* `status` - status vector;
* `src` - source string;
* `srcSize` - size of source string;
* `destBuffer` - buffer for placing the target string;
* `destBufferSize` - size of the buffer for the target string.

The function returns the length of the target string if the conversion was successful.

### Function `convertUtf16ToUtf32`

```cpp
ISC_UINT64 convertUtf16ToUtf32(Status* status, const void* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize)
```

Converts a string from `utf-16` encoding to a string with `utf-32` encoding.

Function parameters:

* `status` - status vector;
* `src` - source string;
* `srcSize` - size of source string;
* `destBuffer` - buffer for placing the target string;
* `destBufferSize` - size of the buffer for the target string.

The function returns the length of the target string if the conversion was successful.

### Function `convertUtf32ToWCS`

```cpp
ISC_UINT64 convertUtf32ToWCS(Status* status, const void* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize)
```

Converts a string from `utf-32` encoding to a string with wide characters (type `wchar_t`).

Function parameters:

* `status` - status vector;
* `src` - source string;
* `srcSize` - size of source string;
* `destBuffer` - buffer for placing the target string;
* `destBufferSize` - size of the buffer for the target string.

The function returns the length of the target string if the conversion was successful.

> [!NOTE]
> **Note**: `wchar_t` is not a platform independent type, its size depends on the operating system on which it is compiled.

### Function `convertUtf32ToUtf8`

```cpp
ISC_UINT64 convertUtf32ToUtf8(StatusType* status, const void* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize)
```

Converts a string from `utf-32` encoding to a string with `utf-8` encoding.

Function parameters:

* `status` - status vector;
* `src` - source string;
* `srcSize` - size of source string;
* `destBuffer` - buffer for placing the target string;
* `destBufferSize` - size of the buffer for the target string.

The function returns the length of the target string if the conversion was successful.

### Function `convertUtf32ToUtf16`

```cpp
ISC_UINT64 convertUtf32ToUtf16(StatusType* status, const void* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize)
```

Converts a string from `utf-32` encoding to a string with `utf-16` encoding.

Function parameters:

* `status` - status vector;
* `src` - source string;
* `srcSize` - size of source string;
* `destBuffer` - buffer for placing the target string;
* `destBufferSize` - size of the buffer for the target string.

The function returns the length of the target string if the conversion was successful.

### Function `convertWCSToUtf8`

```cpp
ISC_UINT64 convertWCSToUtf8(StatusType* status, const void* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize)
```

Recodes a wide character string (of type `wchar_t`) to a string with `utf-8` encoding.

Function parameters:

* `status` - status vector;
* `src` - source string;
* `srcSize` - size of source string;
* `destBuffer` - buffer for placing the target string;
* `destBufferSize` - size of the buffer for the target string.

The function returns the length of the target string if the recoding was successful.

> [!NOTE]
> **Note**: `wchar_t` is not a platform independent type, its size depends on the operating system on which it is compiled.

### Function `convertWCSToUtf16`

```cpp
ISC_UINT64 convertWCSToUtf16(StatusType* status, const void* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize)
```

Recodes a wide character string (of type `wchar_t`) to a string with `utf-16` encoding.

Function parameters:

* `status` - status vector;
* `src` - source string;
* `srcSize` - size of source string;
* `destBuffer` - buffer for placing the target string;
* `destBufferSize` - size of the buffer for the target string.

The function returns the length of the target string if the recoding was successful.

> [!NOTE]
> **Note**: `wchar_t` is not a platform independent type, its size depends on the operating system on which it is compiled.

### Function `convertWCSToUtf32`

```cpp
ISC_UINT64 convertWCSToUtf32(StatusType* status, const void* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize)
```

Recodes a wide character string (of type `wchar_t`) to a string with `utf-32` encoding.

Function parameters:

* `status` - status vector;
* `src` - source string;
* `srcSize` - size of source string;
* `destBuffer` - buffer for placing the target string;
* `destBufferSize` - size of the buffer for the target string.

The function returns the length of the target string if the recoding was successful.

> [!NOTE]
> **Note**: `wchar_t` is not a platform independent type, its size depends on the operating system on which it is compiled.

---

## Interface `IStringConverter`

Interface `IStringConverter` provides functions for converting strings from the encoding bound to an instance of this interface to one of the unicode encodings. To simplify working with this interface, we wrote a high-level wrapper [`StringConverterHelper`](../src/encoding/StringConverterHelper.h).

### Function `getMaxCharSize`

```cpp
int getMaxCharSize()
```

Returns the maximum character size in bytes for the encoding bound to an instance of the `IStringConverter` interface.

### Function `getMinCharSize`

```cpp
int getMinCharSize()
```

Returns the minimum character size in bytes for the encoding bound to an instance of the `IStringConverter` interface.

### Function `getCharsetId`

```cpp
unsigned getCharsetId()
```

Returns the identifier of the Firebird encoding bound to the instance of the `IStringConverter` interface.

Firebird encoding identifiers are listed in the file [`charsets.h`](../src/common/charsets.h).

### Function `getCharsetName`

```cpp
const char* getCharsetName()
```

Returns the name of the Firebird character set bound to the instance of the `IStringConverter` interface.

### `toUtf8` function

```cpp
ISC_UINT64 toUtf8(Status* status, const char* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize)
```

Recodes a string from the encoding bound to an instance of the `IStringConverter` interface to a string with the `utf-8` encoding.

Function parameters:

* `status` - status vector;
* `src` - source string;
* `srcSize` - size of the source string;
* `destBuffer` - buffer for placing the target string;
* `destBufferSize` - size of the buffer for the target string.

The function returns the length of the target string if the recoding was successful.

### `fromUtf8` function

```cpp
ISC_UINT64 fromUtf8(Status* status, const char* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize)
```

Recodes a string from the `utf-8` encoding to a string with the encoding bound to an instance of the `IStringConverter` interface.

Function parameters:

* `status` - status vector;
* `src` - source string;
* `srcSize` - size of the source string;
* `destBuffer` - buffer for placing the target string;
* `destBufferSize` - size of the buffer for the target string.

The function returns the length of the target string if the recoding was successful.

### `toUtf16` function

```cpp
ISC_UINT64 toUtf16(Status* status, const char* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize)
```

Recodes a string from the encoding bound to an instance of the `IStringConverter` interface to a string with the `utf-16` encoding.

Function parameters:

* `status` - status vector;
* `src` - source string;
* `srcSize` - size of the source string;
* `destBuffer` - buffer for placing the target string;
* `destBufferSize` - size of the buffer for the target string.

The function returns the length of the target string if the recoding was successful.

### Function `fromUtf16`

```cpp
ISC_UINT64 fromUtf16(Status* status, const void* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize)
```

Recodes a string from the `utf-16` encoding to a string with the encoding bound to an instance of the `IStringConverter` interface.

Function parameters:

* `status` - status vector;
* `src` - source string;
* `srcSize` - size of the source string;
* `destBuffer` - buffer for placing the target string;
* `destBufferSize` - size of the buffer for the target string.

The function returns the length of the target string if the recoding was successful.

### `toUtf32` function

```cpp
ISC_UINT64 toUtf32(Status* status, const char* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize)
```

Recodes a string from the encoding bound to an instance of the `IStringConverter` interface to a string with the `utf-32` encoding.

Function parameters:

* `status` - status vector;
* `src` - source string;
* `srcSize` - size of the source string;
* `destBuffer` - buffer for placing the target string;
* `destBufferSize` - size of the buffer for the target string.

The function returns the length of the target string if the recoding was successful.

### `fromUtf32` function

```cpp
ISC_UINT64 fromUtf32(Status* status, const void* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize)
```

Recodes a string from the `utf-32` encoding to a string with the encoding bound to an instance of the `IStringConverter` interface.

Function parameters:

* `status` - status vector;
* `src` - source string;
* `srcSize` - size of the source string;
* `destBuffer` - buffer for placing the target string;
* `destBufferSize` - size of the buffer for the target string.

The function returns the length of the target string if the recoding was successful.

### `toWCS` function

```cpp
ISC_UINT64 toWCS(Status* status, const char* src, ISC_UINT64 srcSize, void* destBuffer, ISC_UINT64 destBufferSize)
```

Converts a string from the encoding bound to an instance of the `IStringConverter` interface to a wide-character string (of `wchar_t` type).

Function parameters:

* `status` - status vector;
* `src` - source string;
* `srcSize` - size of source string;
* `destBuffer` - buffer for placing the target string;
* `destBufferSize` - size of the buffer for the target string.

The function returns the length of the target string if the conversion was successful.

> [!NOTE]
> **Note**: `wchar_t` is not a platform-independent type, its size depends on the operating system on which the compilation occurs.

### `fromWCS` function

```cpp
ISC_UINT64 fromWCS(Status* status, const void* src, ISC_UINT64 srcSize, char* destBuffer, ISC_UINT64 destBufferSize)
```

Converts a wide character string (of type `wchar_t`) to a string with the encoding bound to an instance of the `IStringConverter` interface.

Function parameters:

* `status` - status vector;
* `src` - source string;
* `srcSize` - size of the source string;
* `destBuffer` - buffer for placing the destination string;
* `destBufferSize` - size of the buffer for the destination string.

The function returns the length of the target string if the conversion was successful.

> [!NOTE]
> **Note**: `wchar_t` is not a platform-independent type, its size depends on the operating system on which the compilation occurs.

---
