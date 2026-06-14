# Trace Protocol

This document defines the wire protocol used between a host and an embedded device for configuring traceable variables and streaming trace data.

## Frame Encoding

Each message is protected with CRC-CCITT (CRC-16/CCITT-FALSE), then COBS encoded for transport.

COBS appends a `0x00` delimiter to each encoded frame. Receivers can scan incoming bytes until `0x00` to find the end of a frame.

## CRC

The protocol uses CRC-CCITT (CRC-16/CCITT-FALSE).

| Parameter        | Value    |
| ---------------- | -------- |
| Polynomial       | `0x1021` |
| Initial Value    | `0xFFFF` |
| Final XOR        | `0x0000` |
| Input Reflected  | No       |
| Output Reflected | No       |
| Width            | 16 bits  |

The CRC is calculated over the entire message before COBS encoding.

```text
CRC = CRC16_CCITT_FALSE(Message)
```

The CRC is appended in little endian order.

```text
+---------+----------+
| Message | CRC16    |
+---------+----------+
```

Example:

```text
Message

01 05 E8 03 00 00

CRC16

A2 4B

Data Before COBS Encoding

01 05 E8 03 00 00 A2 4B
```

### Transmission

```text
Message
   |
   v
Calculate CRC-CCITT
   |
   v
Append CRC16
   |
   v
COBS Encode
   |
   v
Append 0x00 frame delimiter
   |
   v
Transmit
```

### Reception

```text
Receive bytes until 0x00 frame delimiter
   |
   v
COBS Decode
   |
   v
Extract CRC16
   |
   v
Verify CRC-CCITT
   |
   v
Process Message
```

---

## Message Header

| Offset | Size    | Name        | Description              |
| ------ | ------- | ----------- | ------------------------ |
| 0      | 1 byte  | MessageType | Type of message          |
| 1      | N bytes | Payload     | Message specific payload |

## Message Types

| ID   | Direction      | Name                   |
| ---- | -------------- | ---------------------- |
| 0x01 | Host -> Device | GetConfigRequest       |
| 0x02 | Device -> Host | GetConfigResponse      |
| 0x03 | Host -> Device | GetArrayConfigRequest  |
| 0x04 | Device -> Host | GetArrayConfigResponse |
| 0x05 | Host -> Device | StartTraceRequest      |
| 0x06 | Device -> Host | StartTraceResponse     |
| 0x07 | Device -> Host | TraceData              |
| 0x08 | Host -> Device | StopTraceEvent         |

## Data Types

| ID   | Name    | Size    |
| ---- | ------- | ------- |
| 0x00 | UNKNOWN | 0 bytes |
| 0x01 | CHAR    | 1 byte  |
| 0x02 | UCHAR   | 1 byte  |
| 0x03 | INT8    | 1 byte  |
| 0x04 | UINT8   | 1 byte  |
| 0x05 | INT16   | 2 bytes |
| 0x06 | UINT16  | 2 bytes |
| 0x07 | INT32   | 4 bytes |
| 0x08 | UINT32  | 4 bytes |
| 0x09 | INT64   | 8 bytes |
| 0x0A | UINT64  | 8 bytes |
| 0x0B | FLOAT   | 4 bytes |
| 0x0C | DOUBLE  | 8 bytes |

## GetConfigRequest (0x01)

Direction: `Host -> Device`

Requests the configuration of all traced variables.

### Payload

None.

## GetConfigResponse (0x02)

Direction: `Device -> Host`

Returns all registered variables that are available for tracing.

### Payload

| Offset | Size    | Name          | Type            | Description                                 |
| ------ | ------- | ------------- | --------------- | ------------------------------------------- |
| 0      | 1 byte  | VariableCount | uint8           | Number of variable entries in this response |
| 1      | N bytes | Variables     | VariableEntry[] | Repeated variable entries                   |

### VariableEntry

Each variable entry has the following format:

| Offset | Size    | Name       | Type   | Description                                  |
| ------ | ------- | ---------- | ------ | -------------------------------------------- |
| 0      | 1 byte  | VariableId | uint8  | Internal variable id used by the trace store |
| 1      | 1 byte  | DataType   | uint8  | Variable data type                           |
| 2      | 1 byte  | NameLength | uint8  | Length of the name string                    |
| 3      | N bytes | Name       | string | Variable name without a `\0` terminator      |

The `VariableEntry` structure is repeated `VariableCount` times.

## GetArrayConfigRequest (0x03)

Direction: `Host -> Device`

Requests the configuration of all traced array variables.

### Payload

None.

## GetArrayConfigResponse (0x04)

Direction: `Device -> Host`

Returns all registered array variables that are available for tracing.

### Payload

| Offset | Size    | Name               | Type                 | Description                                       |
| ------ | ------- | ------------------ | -------------------- | ------------------------------------------------- |
| 0      | 1 byte  | ArrayVariableCount | uint8                | Number of array variable entries in this response |
| 1      | N bytes | ArrayVariables     | ArrayVariableEntry[] | Repeated array variable entries                   |

### ArrayVariableEntry

Each array variable entry has the following format:

| Offset | Size    | Name            | Type   | Description                                  |
| ------ | ------- | --------------- | ------ | -------------------------------------------- |
| 0      | 1 byte  | ArrayVariableId | uint8  | Internal variable id used by the trace store |
| 1      | 1 byte  | DataType        | uint8  | Variable data type                           |
| 2      | 2 bytes | ArrayLength     | uint16 | Array length                                 |
| 4      | 1 byte  | NameLength      | uint8  | Length of the name string                    |
| 5      | N bytes | Name            | string | Variable name without a `\0` terminator      |

The `ArrayVariableEntry` structure is repeated `ArrayVariableCount` times.

## StartTraceRequest (0x05)

Direction: `Host -> Device`

Starts tracing selected variables.

### Payload

| Offset | Size    | Name          | Type    | Description                                           |
| ------ | ------- | ------------- | ------- | ----------------------------------------------------- |
| 0      | 1 byte  | VariableCount | uint8   | The amount of variables you want to trace (max of 64) |
| 1      | N bytes | VariableIds   | uint8[] | The ids of all the variables you want to trace        |

## StartTraceResponse (0x06)

Direction: `Device -> Host`

Indicates whether trace acquisition was successfully started.

### Payload

| Offset | Size    | Name             | Type                   | Description                                      |
| ------ | ------- | ---------------- | ---------------------- | ------------------------------------------------ |
| 0      | 1 byte  | MappingCount     | uint8                  | Number of accepted variables in this response    |
| 1      | N bytes | VariableMappings | VariableMappingEntry[] | Repeated accepted variable mappings              |
| ...    | 1 byte  | ErrorCount       | uint8                  | Number of rejected variables in this response    |
| ...    | N bytes | Errors           | ErrorEntry[]           | Repeated rejected variable entries with an error |

Tracing starts when `MappingCount` is greater than zero. The host can infer the result from the counts:

| MappingCount | ErrorCount | Result          |
| ------------ | ---------- | --------------- |
| > 0          | 0          | Full success    |
| > 0          | > 0        | Partial success |
| 0            | > 0        | Full failure    |
| 0            | 0          | Empty request   |

### VariableMappingEntry

Each variable mapping has the following format:

| Offset | Size   | Name            | Type  | Description                                  |
| ------ | ------ | --------------- | ----- | -------------------------------------------- |
| 0      | 1 byte | VariableId      | uint8 | Internal variable id used by the trace store |
| 1      | 1 byte | TraceVariableId | uint8 | Variable id used during stream               |

A different id is used during streaming than during storage, as you can only trace 64 values at a time, but you are able to store up to 256 variables to trace. The host decides what variables need to be traced and thus streamed.

The `VariableMappingEntry` structure is repeated `MappingCount` times.

### ErrorEntry

Each error entry has the following format:

| Offset | Size   | Name                | Type  | Description                      |
| ------ | ------ | ------------------- | ----- | -------------------------------- |
| 0      | 1 byte | VariableId          | uint8 | Requested variable id            |
| 1      | 1 byte | StartTraceErrorCode | uint8 | The error code for this variable |

The `ErrorEntry` structure is repeated `ErrorCount` times.

### StartTraceErrorCode

| Value | Name              | Description                                     |
| ----- | ----------------- | ----------------------------------------------- |
| 0x01  | VariableNotFound  | Requested variable id is not registered         |
| 0x02  | TooManyVariables  | Request exceeded the maximum of 64 trace values |
| 0x03  | DuplicateVariable | Requested variable id was already accepted      |

## TraceData (0x07)

Direction: `Device -> Host`

Contains trace samples generated while tracing is active.
This is a stream of data that is started by the `StartTraceRequest` message and stopped by the `StopTraceEvent` message.

### Payload

| Offset | Size    | Name            | Type             | Description                           |
| ------ | ------- | --------------- | ---------------- | ------------------------------------- |
| 0      | 1 byte  | TraceDataLength | uint8            | The amount of variables that are sent |
| 1      | N bytes | TraceData       | TraceDataEntry[] | The individual variable trace entries |

### TraceDataEntry

Each trace data entry has the following format:

| Offset | Size    | Name            | Type                   | Description                     |
| ------ | ------- | --------------- | ---------------------- | ------------------------------- |
| 0      | 1 byte  | TraceDataHeader | TraceSampleHeaderEntry | Encoded trace sample header     |
| 1      | N bytes | Value           | uint8[]                | The value in bytes of the trace |

The size of `Value` is determined by the `SizeCode` in the `TraceDataHeader`.

### TraceDataHeaderEntry

| Bits | Name       | Description                |
| ---- | ---------- | -------------------------- |
| 7..6 | SizeCode   | Encoded value size         |
| 5..0 | VariableId | Variable id, range `0..63` |

### SizeCode

| Value | Size    |
| ----- | ------- |
| 0     | 1 byte  |
| 1     | 2 bytes |
| 2     | 4 bytes |
| 3     | 8 bytes |

The `TraceDataEntry` structure is repeated `TraceDataLength` times.

## StopTraceEvent (0x08)

Direction: `Host -> Device`

Stops an active trace session. If tracing is already stopped, the device treats this as successful and leaves tracing stopped.

### Payload

None.
