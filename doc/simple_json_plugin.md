# simple_json_plugin plugin

The `simple_json_plugin` plugin is designed to automatically translate binary replication log files into the equivalent JSON format and save the logs to a given directory with the same name, but with a `.json` extension.

Each json file contains a root object consisting of two fields: `header` and `events`.

The `header` field is an object describing the header of the replication segment. It contains the following fields:

* `guid` - database GUID;
* `sequence` - replication segment number;
* `state` - the state the segment is in. Possible values: `free`, `used`, `full`, `archive`;
* `version` - replication log protocol version.

The `events` field is an array of objects, each representing one of the replication events.
The event object can contain the following fields:

* `event` - event type. The following options are available:
  * `SET SEQUENCE` - setting the value of the sequence (generator);
  * `START TRANSACTION` - transaction start;
  * `PREPARE TRANSACTION` - execution of the first phase of confirmation of a two-phase transaction;
  * `SAVEPOINT` - setting a save point;
  * `RELEASE SAVEPOINT` - release a savepoint;
  * `ROLLBACK SAVEPOINT` - rollback savepoint;
  * `COMMIT` - transaction confirmation;
  * `ROLLBACK` - transaction rollback;
  * `INSERT` - inserting a new record into the table;
  * `UPDATE` - updating a record in a table;
  * `DELETE` - deleting a record from a table;
  * `EXECUTE SQL` - execute SQL statement. Such events occur only for DDL statements;
  * `STORE BLOB` - saving BLOB.
* `tnx` - transaction number. Available for all events except `SET SEQUENCE`, because generators operate outside the context of transactions;
* `sequence` is the name of the sequence. Only available in the `SET SEQUENCE` event;
* `value` - the value of the sequence. Only available in the `SET SEQUENCE` event;
* `sql` - SQL query text. Only available in the `EXECUTE SQL` event;
* `blobId` - BLOB identifier. Only available in the `STORE BLOB` event;
* `data` - BLOB segment data in hexadecimal representation. Only available in the `STORE BLOB` event;
* `table` - table name. Available in `INSERT`, `UPDATE` and `DELETE` events;
* `record` - values of record fields. For `INSERT` and `UPDATE` events this is the new entry, and for `DELETE` events it is the old one;
* `oldRecord` - old record field values. Only available in the `UPDATE` event;
* `changedFields` - array of changed field names. Only available in the `UPDATE` event.

Note that for BLOB fields, the BLOB ID is specified as the value.

An example of the contents of a `.json` file:

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

## Plugin setup

Plugin setup example:

```
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

Description of parameters:

* `controlFileDir` - directory in which the control file will be created (by default, the same directory as `sourceDir`);
* `database` - database connection string (mandatory);
* `username` - username to connect to the database;
* `password` - password to connect to the database;
* `plugin` - plugin that handles events that occur during replication log analysis (mandatory);
* `deleteProcessedFile` - whether to delete the log file after processing (default `true`);
* `outputDir` - directory where ready JSON files will be located;
* `dumpBlobs` - whether to publish BLOB field data (`false` by default);
* `register_ddl_events` - whether to register DDL events (`true` by default);
* `register_sequence_events` - whether to register sequence value setting events (`true` by default);
* `include_tables` - a regular expression that defines the names of tables for which you want to track events;
* `exclude_tables` - a regular expression that defines the names of tables for which events should not be tracked.
