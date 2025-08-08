# Service (daemon) fb_streaming

The service (daemon) `fb_streaming` is intended for asynchronous processing of replication events.
It watches for new replication log files in the specified directory, parses them, and generates events that are handled by one of the plugins.

## How it works?

The `fb_streaming` service (daemon) checks the contents of the directory specified for the task in the `fb_streaming.conf` configuration file, and if this directory contains unprocessed replication log files, then it analyzes them and generates appropriate events that are processed by the specified plugin . The last replication segment number processed is written to a control file named `<database guid>`. The location of this file can be specified in the `lockDir` configuration parameter. If this parameter is not specified, then by default the control file `<database guid>` will be created in the directory specified as the archive log directory for the task.

> [!IMPORTANT]
> **Important**: replication log files must be in an archived state.

The control file is necessary to restore operation after a sudden shutdown of the service (for example, the lights were turned off or the server was rebooted). It contains the following information:

- control information (database GUID, size, etc.);
- number of the last processed segment and position in it;
- the number of the segment with the oldest active transaction (a transaction can start in one segment and end in another);
- a list of all active transactions in the form of pairs `{tnxNumber, segmentNumber}`, where `segmentNumber` is the number of the segment in which the transaction began.

If an emergency situation occurs and the `fb_streaming` service has been stopped, then the next time it starts, it reads the control file and rereads all replication segments, starting with the number of the segment with the oldest active transaction, and ending with the number of the last processed segment. In the process of reading these segments, `fb_streaming` repeats all events from the list of active transactions, after which it goes into normal operation.

A replication segment file is deleted if its number is less than the segment number with the oldest active transaction.

During the analysis of replication logs, the following events may occur:

- transaction start (START);
- preparation of a two-phase transaction (PREPARE);
- transaction commit (COMMIT);
- transaction rollback (ROLLBACK);
- set savepoint (SAVE);
- rollback to savepoint (UNDO);
- release savepoint (RELEASE);
- set value of the generator (SET SEQUENCE);
- execution of the SQL statement (EXECUTE SQL). Such events occur only for DDL statements;
- save BLOB (BLOB);
- inserting a new record into the table (INSERT);
- updating records in the table (UPDATE);
- deleting a record from a table (DELETE).

Which of them will be processed and which are simply ignored depends on the selected plugin.

## Installing and starting a service on Windows

Go to the `fb_streaming` installation directory.

Perform the settings in the `fb_streaming.conf` configuration file.

After setting up the configuration file, you can install and start the service.

For help using and installing the service, type the command

```bash
fb_streaming help
```

You will be shown information on installing, removing, starting and stopping the service:

```bash
================================================================================
 Firebird streaming service

 Copyright (c) 2023 IBSurgeon ltd.
================================================================================

usage: fb_streaming <command> [service_name]
Possible commands are:
    install            install service
    remove             remove service
    start              start service
    stop               stop service
```

After setting up the configuration file, you can install and start the service by running the following commands:

```bash
fb_streaming install

fb_streaming start
```

Once installed, you can manage the service through the graphical utility - services (`services.msc`).

To remove a service, run the following commands:

```bash
fb_streaming stop

fb_streaming remove
```

> [!NOTE]
> **Note**: If you have multiple `fb_streaming` services running, they must be given unique names when installed, started, stopped, and removed.

## Installing and starting a service on Linux

Go to the `fb_streaming` installation directory. On Linux this is usually the `/opt/fb_streaming` directory.

Perform the settings in the `fb_streaming.conf` configuration file.

After setting up the configuration file, you can install and start the service by running the following commands:

```bash
sudo systemctl enable fb_streaming

sudo systemctl start fb_streaming
```

To remove a service, run the following commands:

```bash
sudo systemctl stop fb_streaming

sudo systemctl disable fb_streaming
```

## Configuring the service (daemon) fb_streaming

The service is configured using the `fb_streaming.conf` file, which must be located in the same directory as the `fb_streaming` executable file.

A single instance of a service can serve multiple tasks at once. Each task runs in its own thread. A separate task is created for each directory with replication log files.
Tasks are configured as follows:

```
# logLevel = info

# pluginDir = $(root)/stream_plugins 

task = <sourceDir_1>
{
# other parameters of task 1
}

task = <sourceDir_2>
{
# other parameters of task 2
}
```

General parameters of the `fb_streaming` service (daemon):

- `logLevel` - logging level (default info). Allowed logging levels: trace, debug, info, warning, error, critical, off.
- `pluginDir` - directory where `fb_streaming` plugins are located. Defaults to `$(root)/stream_plugins`. The macro substitution `$(root)` denotes the root directory of the service (daemon).

Next are the task configurations. One service instance can serve several tasks at once. Each task runs in its own thread.
A separate task is created for each directory with replication log files.

Here `sourceDir_N` is the directory with replication log files. These directories must be unique. The same directory cannot be processed by more than one task.

The main parameters of the task:

- `controlFileDir` - the directory in which the control file will be created (by default, the same directory as `sourceDir`);
- `errorTimeout` - timeout after error in seconds. After this timeout expires, the segments will be re-scanned and the task will be restarted. The default is 60 seconds;
- `database` - database connection string (required);
- `username` - user name for connecting to the database;
- `password` - password for connecting to the database;
- `plugin` - plugin that processes events that occur during the analysis of the replication log (required);
- `deleteProcessedFile` - whether to delete the log file after processing (true by default).

The task may also contain other parameters specific to the plugin being used.

Plugins for handling events from the replication log are located in dynamic libraries.
The name of the dynamic library file depends on the operating system and is built as follows:

- for Windows `<plugin name>.dll`
- for Linux `lib<plugin name>.so`

The dynamic library file must be located in the `stream_plugins` directory, or in the one specified in the `pluginDir` parameter.

## Firebird configuration tricks

Please note: replication log archives folder should only be processed by a single `fb_streaming` service task.
If you want to have logical replication running at the same time, or multiple services running on the log archive,
then duplication of archive logs in different directories is necessary.

This can be done in the `replication.conf` file as follows

```
journal_archive_directory = <archiveDir>
journal_archive_command = "copy $(pathname) $(archivepathname) && copy $(pathname) <archiveDirTask>
```

Here `archiveDir` is the directory of archives for asynchronous replication, `archiveDirTask` is the directory of archives for the service task `fb_streaming`.

## Plugins

The `fb_streaming` service (daemon) comes bundled with a demo plugin `simple_json_plugin`.
We can write other plugins to order. We have ready-made plugins:

- `kafka_cdc_plugin` - Recording data change events in Kafka (Change Data Capture);
- `mongodb_events_plugin` - recording replication events in MongoDB;
- `rabbitmq_plugin` - placing replication events in the RabbitMQ queue server;
- `fts_lucene_plugin` - updating full-text indexes created using LuceneFTS UDR.

## Plugin simple_json_plugin

The `simple_json_plugin` plugin is designed to automatically translate binary replication log files into an equivalent JSON format and save the logs to a specified directory with the same name, but with a `.json` extension.

Each json file contains a root object consisting of two fields: `header` and `events`.

The `header` field is an object describing the header of the replication segment. It contains the following fields:

- `guid` - database GUID;
- `sequence` - replication segment number;
- `state` - the state in which the segment is located. Possible values: `free`, `used`, `full`, `archive`;
- `version` - replication log version.

The `events` field is an array of objects, each of which represents one of the replication events.
The event object can contain the following fields:

- `event` - event type. The following options are available:
  - `SET SEQUENCE` - setting the sequence (generator) value;
  - `START TRANSACTION` - start of a transaction;
  - `PREPARE TRANSACTION` - execution of the first phase of confirmation of a two-phase transaction;
  - `SAVEPOINT` - setting a save point;
  - `RELEASE SAVEPOINT` - releasing the save point;
  - `ROLLBACK SAVEPOINT` - rollback the save point;
  - `COMMIT` - transaction confirmation;
  - `ROLLBACK` - rollback a transaction;
  - `INSERT` - inserting a new record into the table;
  - `UPDATE` - updating a record in the table;
  - `DELETE` - deleting a record from the table;
  - `EXECUTE SQL` - execution of an SQL statement. Such events only occur for DDL statements;
  - `STORE BLOB` - saving BLOB.
- `tnx` - transaction number. Available for all events except `SET SEQUENCE`, since generators operate outside the transaction context;
- `sequence` - sequence name. Available only in the `SET SEQUENCE` event;
- `value` - the value of the sequence. Available only in the `SET SEQUENCE` event;
- `sql` - SQL query text. Available only in the `EXECUTE SQL` event;
- `blobId` - BLOB identifier. Available only in the `STORE BLOB` event;
- `data` - BLOB segment data in hexadecimal notation. Available only in the `STORE BLOB` event;
- `table` - table name. Available in `INSERT`, `UPDATE` and `DELETE` events;
- `record` - record field values. For `INSERT` and `UPDATE` events this is a new record, and for `DELETE` it is an old one;
- `oldRecord` - values of the fields of the old record. Available only in the `UPDATE` event;
- `changedFields` - an array of names of changed fields. Available only in the `UPDATE` event.

Please note that for BLOB fields the BLOB ID is specified as the value.

Example contents of a `.json` file:

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

### Plugin settings

Plugin configuration example:

```
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

Description of parameters:

- `controlFileDir` - the directory in which the control file will be created (by default, the same directory as `sourceDir`);
- `database` - database connection string (required);
- `username` - user name to connect to the database;
- `password` - password for connecting to the database;
- `plugin` - plugin that processes events that occur during the analysis of the replication log (required);
- `deleteProcessedFile` - whether to delete the log file after processing (default `true`);
- `outputDir` - the directory in which the finished JSON files will be located (required);
- `dumpBlobs` - whether to publish data from BLOB fields (false by default);
- `register_ddl_events` - whether to register DDL events (true by default);
- `register_sequence_events` - whether to register sequence value setting events (true by default);
- `include_tables` - a regular expression that defines the names of the tables for which events need to be monitored;
- `exclude_tables` - a regular expression that defines the names of tables for which events do not need to be monitored.
