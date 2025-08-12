# Firebird Streaming Examples

Here are examples of writing plugins for Firebird streaming technology.

Firebird streaming is a technology for asynchronous publication of events that occur during the analysis of the replication log.

The `fb_streaming` service (daemon) is used to process events. The service monitors new replication log files, analyzes them, and generates events that are processed by one of the plugins. A detailed description of the `fb_streaming` service can be found in the document [fb_streaming Service](doc/fb_streaming.md).

In addition, there is a special utility `fb_repl_print`, which allows you to view the contents of the specified replication log file, which facilitates debugging of plugins and the replication itself. A detailed description of the `fb_repl_print` utility can be found in the document [fb_repl_print Utility](doc/fb_repl_print.md).

To obtain binary files of the `fb_streaming` service and the `fb_repl_print` utility, please contact IBSurgeon.

The process of writing your own plugins is described in detail in the document [Writing Your Own Plugin for the fb_streaming Service](doc/writing_plugin.md).

As an example of a plugin, let's consider a simple library that will translate the original binary replication log files into an equivalent JSON format and save the logs in a specified directory with the same name but with a `.json` extension. Let's call this plugin `simple_json_plugin`.

You can download the compiled example with configuration from the links:

* [SimpleJsonPlugin_Win_x64.zip](https://github.com/IBSurgeon/FBStreamingExamples/releases/download/1.5/simple_json_plugin-1.5.3-Win-x64.zip)
* [SimpleJsonPlugin_Win_x86.zip](https://github.com/IBSurgeon/FBStreamingExamples/releases/download/1.5/simple_json_plugin-1.5.3-Win-x86.zip)

## Description of the simple_json_plugin

The `simple_json_plugin` is designed for the automatic translation of binary replication log files into an equivalent JSON format 
and to save the logs in a specified directory with the same name but with a `.json` extension.

Each JSON file contains a root object consisting of two fields: `header` and `events`.

The `header` field is an object describing the header of the replication segment. It contains the following fields:

* `guid` - database GUID;
* `sequence` - replication segment number;
* `state` - the state of the segment. Possible values: `free`, `used`, `full`, `archive`;
* `version` - replication log protocol version.

The `events` field is an array of objects, each representing one of the replication events. 
An event object may contain the following fields:

* `event` - event type. The following options are available:
  - `SET SEQUENCE` - setting the value of a sequence (generator);
  - `START TRANSACTION` - starting a transaction;
  - `PREPARE TRANSACTION` - executing the first phase of confirming a two-phase transaction;
  - `SAVEPOINT` - setting a savepoint;
  - `RELEASE SAVEPOINT` - releasing a savepoint;
  - `ROLLBACK SAVEPOINT` - rolling back a savepoint;
  - `COMMIT` - committing a transaction;
  - `ROLLBACK` - rolling back a transaction;
  - `INSERT` - inserting a new record into a table;
  - `UPDATE` - updating a record in a table;
  - `DELETE` - deleting a record from a table;
  - `EXECUTE SQL` - executing an SQL statement. Such events only occur for DDL statements;
  - `STORE BLOB` - storing a BLOB.
* `tnx` - transaction number. Available for all events except `SET SEQUENCE`, as generators operate outside the context of transactions;
* `sequence` - sequence name. Available only in the `SET SEQUENCE` event;
* `value` - sequence value. Available only in the `SET SEQUENCE` event;
* `sql` - SQL query text. Available only in the `EXECUTE SQL` event;
* `blobId` - BLOB identifier. Available only in the `STORE BLOB` event;
* `data` - BLOB segment data in hexadecimal representation. Available only in the `STORE BLOB` event;
* `table` - table name. Available in the `INSERT`, `UPDATE`, and `DELETE` events;
* `record` - record field values. For `INSERT` and `UPDATE` events, this is the new record, and for `DELETE`, it is the old one;
* `oldRecord` - values of the old record fields. Available only in the `UPDATE` event;
* `changedFields` - an array of names of changed fields. Available only in the `UPDATE` event.

Note that for BLOB fields, the value is indicated as a BLOB identifier.

Example of a `.json` file content:

```json
{
    "events": [
        {
            "event": "START TRANSACTION",
            "tnx": 6259
        },
        {
            "event": "SAVEPOINT",
            "tnx": 6259
        },
        {
            "event": "SAVEPOINT",
            "tnx": 6259
        },
        {
            "changedFields": [
                "SHORTNAME_EN"
            ],
            "event": "UPDATE",
            "oldRecord": {
                "CODE_COLOR": 3,
                "CODE_SENDER": 1,
                "NAME": "красно-серая",
                "NAME_DE": "",
                "NAME_EN": "red grey",
                "SHORTNAME": "кр.-сер.",
                "SHORTNAME_EN": ""
            },
            "record": {
                "CODE_COLOR": 3,
                "CODE_SENDER": 1,
                "NAME": "красно-серая",
                "NAME_DE": "",
                "NAME_EN": "red grey",
                "SHORTNAME": "кр.-сер.",
                "SHORTNAME_EN": "fff"
            },
            "table": "COLOR",
            "tnx": 6259
        },
        {
            "event": "RELEASE SAVEPOINT",
            "tnx": 6259
        },
        {
            "event": "RELEASE SAVEPOINT",
            "tnx": 6259
        },
        {
            "event": "COMMIT",
            "tnx": 6259
        }
    ],
    "header": {
        "guid": "{AA08CB53-C875-4CA3-B513-877D0668885D}",
        "sequence": 3,
        "state": "archive",
        "version": 1
    }
}
```
### SimpleJsonPlugin Configuration

Example of plugin configuration:

```conf
task = d:\fbdata\4.0\replication\testdb\archive
{
    deleteProcessedFile = true
    database = inet://localhost:3054/test
    username = SYSDBA
    password = masterkey
    plugin = simple_json_plugin
    dumpBlobs = true
    register_ddl_events = true
    register_sequence_events = true
    outputDir = d:\fbdata\4.0\replication\testdb\json_archive
    # include_tables = 
    # exclude_tables = 
}
```

Parameter descriptions:

* `controlFileDir` - the directory where the control file will be created (by default, the same directory as `sourceDir`);
* `database` - database connection string (mandatory);
* `username` - username for database connection;
* `password` - password for database connection;
* `plugin` - the plugin that processes events arising during the analysis of the replication log (mandatory);
* `deleteProcessedFile` - whether to delete the log file after processing (default `true`);
* `outputDir` - the directory where the finished JSON files will be located;
* `dumpBlobs` - whether to publish BLOB field data (default false);
* `register_ddl_events` - whether to register DDL events (default true);
* `register_sequence_events` - whether to register sequence value setting events (default true);
* `include_tables` - regular expression defining the names of tables for which events should be tracked;
* `exclude_tables` - regular expression defining the names of tables for which events should not be tracked.
