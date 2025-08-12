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
    "header": {
        "version": 1,
        "guid": "{F396449D-F6E4-4812-875E-248AB7C2BEE7}",
        "sequence": 1,
        "state": "archive"
    },
    "events": [
        {
            "event": "START TRANSACTION",
            "tnx": 1937
        },
        {
            "event": "SAVEPOINT",
            "tnx": 1937
        },
        {
            "event": "SAVEPOINT",
            "tnx": 1937
        },
        {
            "event": "UPDATE",
            "table": "CUSTOMER",
            "tnx": 1937,
            "changedFields": [
                "ADDRESS"
            ],
            "oldRecord": {
                "CUSTOMER_ID": 8,
                "NAME": "Abigail Thomas",
                "ADDRESS": null,
                "ZIPCODE": null,
                "PHONE": "1-290-853-7531"
            },
            "record": {
                "CUSTOMER_ID": 8,
                "NAME": "Abigail Thomas",
                "ADDRESS": "555",
                "ZIPCODE": null,
                "PHONE": "1-290-853-7531"
            }
        },
        {
            "event": "RELEASE SAVEPOINT",
            "tnx": 1937
        },
        {
            "event": "RELEASE SAVEPOINT",
            "tnx": 1937
        },
        {
            "event": "COMMIT",
            "tnx": 1937
        }
    ]
}
```

### Database for examples

For the examples we will use the database with ODS 13.0 (Firebird 4.0) or ODS 13.1 (Firebird 5.0), which you can download from the following links:

* [example-db_4_0](https://github.com/sim1984/example-db_4_0)
* [example-db_5_0](https://github.com/sim1984/example-db_5_0)

### Setting up the `fb_streaming` service and the `simple_json_plugin` plugin

First of all, download the `simple_json_plugin` plugin and place `simple_json_plugin.dll` in the `$(fb_streaming_root)/stream_plugins` directory, where `$(fb_streaming_root)` is the root directory of the `fb_streaming` service installation.

Next, set up the `fb_streaming.conf` configuration so that `fb_streaming` automatically publishes changes to json files.

Example of plugin configuration:

```conf
task = d:\fbdata\4.0\replication\examples\json_source
{
	deleteProcessedFile = true
	database = inet://localhost:3054/examples
	username = SYSDBA
	password = masterkey
	plugin = simple_json_plugin
	dumpBlobs = true
	register_ddl_events = true
	register_sequence_events = true
	outputDir = d:\fbdata\4.0\replication\examples\json_archive
	# include_tables = 
	# exclude_tables = 
}
```

The `task` parameter describes the task to be performed by the `fb_streaming` service. It specifies the folder where the replication segment files are located for processing by the plugin. There may be several such tasks. This parameter is complex and itself describes the configuration of a specific task. Let's describe the parameters available for the task performed by the `simple_json_plugin` plugin:

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
