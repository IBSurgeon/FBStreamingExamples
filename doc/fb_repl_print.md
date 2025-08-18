# fb_repl_print utility

The `fb_repl_print` utility allows you to view the contents of a specified replication log file in a human-readable form.
The replication logs themselves are in binary format, so it is impossible to analyze them without a special utility.

The following events may appear in the replication log file:

- transaction start (START);
- preparation of a two-phase transaction (PREPARE);
- transaction commit (COMMIT);
- transaction rollback (ROLLBACK);
- set a savepoint (SAVE);
- rollback to savepoint (UNDO);
- release savepoint (RELEASE);
- set value of the generator (SET SEQUENCE);
- execution of the SQL statement (EXECUTE SQL). Such events occur only for DDL statements;
- save BLOB (BLOB);
- inserting a new record into the table (INSERT);
- updating records in the table (UPDATE);
- deleting a record from a table (DELETE).

Information about these events will be printed out by the `fb_repl_print` utility. In addition, using  additional switches, you can also display information about the replication file header, BLOB content (in hexadecimal), and field values for new (INSERT, UPDATE events) and old (UPDATE, DELETE events) records.

To get help about utility options, type the command

```bash
fb_repl_print --help
```

As a result, the following will be displayed on the console:

```
Usage fb_repl_print [file_path] <options>
General options:
    -h [ --help ]                        Show help
    -z [ --version ]                     Show version
    -i [ --input-file ] file_path        Path to the replication log file
                                            (required)
    -H [ --print-header ]                Print segment header
    -B [ --print-blobs ]                 Print BLOBs
    -R [ --dump-records ]                Dump table records
                                            (require --database option)
    -F [ --print-format ]                Print record format
                                            (require --dump-records option)

Database options:
    -d [ --database ] connection_string  Database connection string
    -u [ --username ] user               User name
    -p [ --password ] password           Password
```

The only required parameter for the `fb_repl_print` utility to work is the path to the replication log file. It can be given either as the first argument or with the `--input-file` or `-i` options. In this mode, the utility will display information about all events present in the replication log file.

Usage example:

```
fb_repl_print "d:\fbdata\test\archive\TEST.FDB.journal-000000001"

BLOCK (offset: 48)
[200] START (offset: 64)
[200] SAVE (offset: 65)
[200] SAVE (offset: 66)
[200] INSERT CUSTOMERS (length: 3082) (offset: 78)
[200] RELEASE (offset: 3169)
[200] RELEASE (offset: 3170)
BLOCK (offset: 3171)
[] SET SEQUENCE RDB$2 = 1 (offset: 3194)
[200] COMMIT (offset: 3207)
```

The number `200` in square brackets is the number of the transaction in which the actions are performed. If the square brackets are empty, then the action occurs outside the transaction control.

`offset` denotes the offset in bytes relative to the start of the segment file. This information can help in troubleshooting replication errors.

If you want the replication log file header information to be present, add the `-H` or `--print-header` option.
Header information includes:

- segment file name;
- replication protocol version;
- unique database identifier (Guid);
- sequence number (replication log file number);
- replication segment status;
- replication segment size in bytes.

Example of outputting the contents of a replication log file with a header

```
fb_repl_print "d:\fbdata\test\archive\TEST.FDB.journal-000000001" --print-header

================================================================================
Segment: TEST.FDB.journal-000000001
Version: 1
Guid: {9D66A972-A8B9-42E0-8542-82D1DA5F1692}
Sequence: 1
State: archive
Length: 3208
================================================================================


BLOCK (offset: 48)
[200] START (offset: 64)
[200] SAVE (offset: 65)
[200] SAVE (offset: 66)
[200] INSERT CUSTOMERS (length: 3082) (offset: 78)
[200] RELEASE (offset: 3169)
[200] RELEASE (offset: 3170)
BLOCK (offset: 3171)
[] SET SEQUENCE RDB$2 = 1 (offset: 3194)
[200] COMMIT (offset: 3207)
```

If the table contains BLOB fields, then during the INSERT and UPDATE operations, the new values of these fields are written to the database before the record is inserted or updated. Then, when a record is directly inserted or updated, a reference to the previously stored value is written to the BLOB field. These actions are  also duplicated in the replication log. To view the contents of these saved BLOBs, use the `-B` or `--print-blobs` switch.

An example of printing a BLOB from the replication log:

```
fb_repl_print "d:\fbdata\4.0\replication\fts_demo\archive\FTS_DEMO.FDB.journal-000000003" -B

BLOCK (offset: 48)
[605] SAVE (offset: 65)
[605] SAVE (offset: 66)
[605] BLOB 131:1110727 (length 113) (offset: 67)
4d616b652073757265207468697320666974732062792065...
[605] BLOB 131:1110727 (length 0) (offset: 191)
[605] BLOB 131:1110728 (length 102) (offset: 193)
4153494e3a423030425941515954517c5368697070696e67...
[605] BLOB 131:1110728 (length 0) (offset: 306)
[605] BLOB 131:1110729 (length 802) (offset: 308)
476f20746f20796f7572206f726465727320616e64207374617274...
[605] BLOB 131:1110729 (length 0) (offset: 1121)
[605] INSERT PRODUCTS (length: 4539) (offset: 1133)
[605] RELEASE (offset: 5681)
[605] RELEASE (offset: 5682)
BLOCK (offset 5683)
[] SET SEQUENCE RDB$7 = 10022 (offset: 5686)
[605] COMMIT (offset: 5699)
```

Let's decode the string `[605] BLOB 131:1110727 (length 113)`. The number `605` is the transaction number, followed by the event name `BLOB`, which means save the BLOB. Next, in parentheses, the length of the BLOB in bytes is indicated. The next line is the contents of the BLOB (byte order) in hexadecimal notation. The BLOB save event ends with the same string it started with, but with zero length.

> [!NOTE]
> In the examples, the output of BLOB fields is shortened, in fact, there is no ellipsis in the output. All bytes of BLOB fields are printed.

Along with the INSERT, UPDATE, and DELETE events, a new and old entry is always kept in the replication log. For INSERT event - new record, for UPDATE - new and old record, for DELETE - old record. Table entries are stored in a format very similar to how they are stored directly in the database. In order to decipher the values of the record fields, information about the format used is required. The record formats in the database are stored in the `RDB$FORMATS` table. In HQBird 2.5 and HQBird 3.0, the replication log also contains information about the record format for the current operation (the so-called logical record format). However, logical formats were not included in the Firebird 4.0 (HQBird 4.0) protocol. In principle, to apply changes from the replication log on the replica side, you can do without them, since at the time of applying the INSERT/UPDATE/DELETE operations. The DDL statement that changed the format must already be executed on the side of the replica, and therefore its format is known. However, in rare cases, this rule does not work, and therefore failures may occur. This may change in future versions.

To display new and old record column values in INSERT, UPDATE, and DELETE events, you must specify the `-R` or `--dump-records` switch. For the reasons described above, this switch also requires a number of other options in order to connect to the database and read the record format information. The database connection is always made with the UTF8 character set.

An example of displaying the values of record fields:

```
fb_repl_print "d:\fbdata\4.0\replication\fts_demo\archive\FTS_DEMO.FDB.journal-000000003" -B -R -d inet://localhost:3054/fts_demo -u SYSDBA -p masterkey

BLOCK (offset: 48)
[698] START (offset: 64)
[698] SAVE (offset: 65)
[698] SAVE (offset: 66)
[698] BLOB 131:1920 (length 315) (offset: 67)
4d616b6520737572652074686973206669747320627920656e74...
[698] BLOB 131:1920 (length 0) (offset: 393)
[698] BLOB 131:1921 (length 315) (offset: 395)
50726f6475637444696d656e73696f6e733a3578307834696e636865737c49...
[698] BLOB 131:1921 (length 0) (offset: 721)
[698] BLOB 131:1922 (length 239) (offset: 723)
73686f7720757020746f203220726576696577732062792064656661756c742...
[698] BLOB 131:1922 (length 0) (offset: 973)
[698] UPDATE PRODUCTS (orgLength: 4539, newLength: 4539) (offset: 985)

Old Data:
{
    "PRODUCT_ID": 15,
    "PRODUCT_UUID": "1ecccb43e0f5c0162218371916ffa553",
    "ABOUT_PRODUCT": "131:1920",
    "PRODUCT_SPECIFICATION": "131:1921",
    "TECHNICAL_DETAILS": "131:1922",
    "VARIANTS": null,
    "PRODUCT_NAME": "C&D Visionary DC Comic Originals Flash Logo Sticker",
    "UPC_EAN_CODE": null,
    "SELLING_PRICE": "$4.99",
    "MODEL_NUMBER": "S-DC-0140",
    "SHIPPING_WEIGHT": "0.16 ounces",
    "PRODUCT_DIMENSIONS": null,
    "PRODUCT_URL": "https://www.amazon.com/Visionary-Comic-Originals-Flash-Sticker/dp/B00U87LZ0Y",
    "IS_AMAZON_SELLER": true
}

New Data:
{
    "PRODUCT_ID": 15,
    "PRODUCT_UUID": "1ecccb43e0f5c0162218371916ffa553",
    "ABOUT_PRODUCT": "131:1920",
    "PRODUCT_SPECIFICATION": "131:1921",
    "TECHNICAL_DETAILS": "131:1922",
    "VARIANTS": null,
    "PRODUCT_NAME": "C&D Visionary DC Comic Originals Flash Logo Sticker",
    "UPC_EAN_CODE": null,
    "SELLING_PRICE": "$4.99",
    "MODEL_NUMBER": "S-DC-0140",
    "SHIPPING_WEIGHT": "0.17 ounces",
    "PRODUCT_DIMENSIONS": null,
    "PRODUCT_URL": "https://www.amazon.com/Visionary-Comic-Originals-Flash-Sticker/dp/B00U87LZ0Y",
    "IS_AMAZON_SELLER": true
}
[698] RELEASE (offset: 10076)
[698] RELEASE (offset: 10077)
BLOCK (offset 10078)
[698] COMMIT (offset: 10094)
```

The value of the record fields is displayed in JSON format. All strings are output as is, except for strings with the character set `OCTETS` (or type `[VAR]BINARY`), which are output in hexadecimal notation. For BLOB fields, an identifier is displayed instead of their value, by this identifier you can find the BLOB values, since they were saved earlier. Note that the field BLOB ID values are there for both new and old records, while the BLOB values themselves are stored only for new records.

> [!NOTE]
> The utility always outputs the contents of replication log files in UTF8 encoding. This means that in order to display string characters correctly, you must switch the encoding of your console. On Windows, this is done with the `chcp 65001` command.

The required record format, and the table itself, cannot always be found in the database. For example, you could issue the `DROP TABLE` command, which, of course, was written to some replication log, but you use the `fb_repl_print` utility to view the log file in which this table already existed.

If the table or format for decoding the values of the record fields is not found, then `fb_repl_print` will issue an appropriate warning.

If you need to see the record format (table field names and their types), add the `-F` or `--print-format` switch.

```
fb_repl_print "d:\fbdata\4.0\replication\fts_demo\archive\FTS_DEMO.FDB.journal-000000004" -B -R -F -d inet://localhost:3054/fts_demo -u SYSDBA -p masterkey

BLOCK (offset: 48)
[698] START (offset: 64)
[698] SAVE (offset: 65)
[698] SAVE (offset: 66)
[698] BLOB 131:1920 (length 315) (offset: 67)
4d616b6520737572652074686973206669747320627920656e746572696e6720796f757...
[698] BLOB 131:1920 (length 0) (offset: 393)
[698] BLOB 131:1921 (length 315) (offset: 395)
50726f6475637444696d656e73696f6e733a3578307834696e636865737c4974656d576....
[698] BLOB 131:1921 (length 0) (offset: 721)
[698] BLOB 131:1922 (length 239) (offset: 723)
73686f7720757020746f203220726576696577732062792064656661756c742043264420...
[698] BLOB 131:1922 (length 0) (offset: 973)
[698] UPDATE PRODUCTS (orgLength: 4539, newLength: 4539) (offset: 985)

Record format (fields: 14):
--------------------------------------------------------------------------------
  Id                            Name                  Type         Character set
--------------------------------------------------------------------------------
   0                      PRODUCT_ID                BIGINT
   1                    PRODUCT_UUID            BINARY(16)
   2                   ABOUT_PRODUCT    BLOB SUB_TYPE TEXT                  UTF8
   3           PRODUCT_SPECIFICATION    BLOB SUB_TYPE TEXT                  UTF8
   4               TECHNICAL_DETAILS    BLOB SUB_TYPE TEXT                  UTF8
   5                        VARIANTS    BLOB SUB_TYPE TEXT                  UTF8
   6                    PRODUCT_NAME          VARCHAR(200)                  UTF8
   7                    UPC_EAN_CODE          VARCHAR(150)                  UTF8
   8                   SELLING_PRICE          VARCHAR(400)                  UTF8
   9                    MODEL_NUMBER           VARCHAR(45)                  UTF8
  10                 SHIPPING_WEIGHT           VARCHAR(15)                  UTF8
  11              PRODUCT_DIMENSIONS           VARCHAR(50)                  UTF8
  12                     PRODUCT_URL          VARCHAR(255)                  UTF8
  13                IS_AMAZON_SELLER               BOOLEAN
--------------------------------------------------------------------------------

Old Data:
{
    "PRODUCT_ID": 15,
    "PRODUCT_UUID": "1ecccb43e0f5c0162218371916ffa553",
    "ABOUT_PRODUCT": "131:1920",
    "PRODUCT_SPECIFICATION": "131:1921",
    "TECHNICAL_DETAILS": "131:1922",
    "VARIANTS": null,
    "PRODUCT_NAME": "C&D Visionary DC Comic Originals Flash Logo Sticker",
    "UPC_EAN_CODE": null,
    "SELLING_PRICE": "$4.99",
    "MODEL_NUMBER": "S-DC-0140",
    "SHIPPING_WEIGHT": "0.16 ounces",
    "PRODUCT_DIMENSIONS": null,
    "PRODUCT_URL": "https://www.amazon.com/Visionary-Comic-Originals-Flash-Sticker/dp/B00U87LZ0Y",
    "IS_AMAZON_SELLER": true
}

New Data:
{
    "PRODUCT_ID": 15,
    "PRODUCT_UUID": "1ecccb43e0f5c0162218371916ffa553",
    "ABOUT_PRODUCT": "131:1920",
    "PRODUCT_SPECIFICATION": "131:1921",
    "TECHNICAL_DETAILS": "131:1922",
    "VARIANTS": null,
    "PRODUCT_NAME": "C&D Visionary DC Comic Originals Flash Logo Sticker",
    "UPC_EAN_CODE": null,
    "SELLING_PRICE": "$4.99",
    "MODEL_NUMBER": "S-DC-0140",
    "SHIPPING_WEIGHT": "0.17 ounces",
    "PRODUCT_DIMENSIONS": null,
    "PRODUCT_URL": "https://www.amazon.com/Visionary-Comic-Originals-Flash-Sticker/dp/B00U87LZ0Y",
    "IS_AMAZON_SELLER": true
}
[698] RELEASE (offset: 10076)
[698] RELEASE (offset: 10077)
BLOCK (offset 10078)
[698] COMMIT (offset: 10094)
```

## Finding errors in replication segments

Suppose that an error occurs while applying changes to a replica. In this case, the `replication.log` usually records the reason for the error.
It might contain something like

```
fb-repl-server (replica) Mon Jul 29 13:15:05 2024
    Database: /var/fbdata/mydb.fdb
    ERROR: violation of FOREIGN KEY constraint "FK_TRIAL_LINE_REF_HORSE" on table "TRIAL_LINE"
    Foreign key reference target does not exist
    Problematic key value is ("CODE_HORSE" = 12345)
    At segment 106, offset 130012
```

In this case, you can look at the contents of segment `106` using `fb_repl_print` and find the location of the error by `offset` equal to `130012`.

```
fb_repl_print "/var/fbdata/replication/archive/MYDB.FDB.journal-000000106" -B -R -d inet://localhost:mydb -u SYSDBA -p masterkey

...
BLOCK (offset: 130012)
[698] START (offset: 130013)
[698] SAVE (offset: 130014)
[698] SAVE (offset: 130015)
[698] INSERT TRIAL_LINE (length: 4539) (offset: 130016)

Data:
{
    "CODE_TRIAL_LINE": 155674870,
    "CODE_HORSE": 12345,
    ...
}
...

```
