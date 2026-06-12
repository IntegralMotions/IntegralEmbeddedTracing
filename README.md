# Trace Protocol

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
   ↓
Calculate CRC-CCITT
   ↓
Append CRC16
   ↓
COBS Encode
   ↓
Transmit
```

### Reception

```text
Receive
   ↓
COBS Decode
   ↓
Extract CRC16
   ↓
Verify CRC-CCITT
   ↓
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
| 0x08 | Host -> Device | StopTraceRequest       |
| 0x09 | Device -> Host | StopTraceResponse      |

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

| Offset | Size    | Name       | Type   | Description                                              |
| ------ | ------- | ---------- | ------ | -------------------------------------------------------- |
| 0      | 1 byte  | VariableId | uint8  | Internal variable id used in the trace variable store    |
| 1      | 1 byte  | DataType   | uint8  | Variable data type                                       |
| 2      | 1 byte  | NameLength | uint8  | Length of the name string, excluding the `\0` terminator |
| 3      | N bytes | Name       | string | Null terminated variable name                            |

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
| 1      | N bytes | ArrayVariables     | ArrayVariableEntry[] | Repeated variable entries                         |

### ArrayVariableEntry

Each array variable entry has the following format:

| Offset | Size    | Name            | Type   | Description                                              |
| ------ | ------- | --------------- | ------ | -------------------------------------------------------- |
| 0      | 1 byte  | ArrayVariableId | uint8  | Internal variable id used in the trace variable store    |
| 1      | 1 byte  | DataType        | uint8  | Variable data type                                       |
| 1      | 1 byte  | ArrayLength     | uint8  | Array length                                             |
| 2      | 1 byte  | NameLength      | uint8  | Length of the name string, excluding the `\0` terminator |
| 3      | N bytes | Name            | string | Null terminated variable name                            |

The `ArrayVariableEntry` structure is repeated `ArrayVariableCount` times.

## StartTraceRequest (0x05)

Direction: `Host -> Device`

Starts tracing selected variables.

### Payload

| Offset | Size   | Name          | Type    | Description                                           |
| ------ | ------ | ------------- | ------- | ----------------------------------------------------- |
| 0      | 1 byte | VariableCount | uint8   | The amount of variables you want to trace (max of 64) |
| 1      | N byte | VariableIds   | uint8[] | The ids of all the variables you want to trace        |


## StartTraceResponse (0x06)

Direction: `Device -> Host`

Indicates whether trace acquisition was successfully started.

### Payload

| Offset | Size   | Name                                      | Type                                     | Description                                                                                 |
| ------ | ------ | ----------------------------------------- | ---------------------------------------- | ------------------------------------------------------------------------------------------- |
| 0      | 1 byte | Status                                    | uint8                                    | 0 = Success, non-zero = Error                                                               |
| 1      | 1 byte | ArrayLength                               | uint8                                    | The length of the upcoming array                                                            |
| 2      | N byte | VariableMapping `or` ErroredVariableEntry | VariableMappingEntry[] `or` ErrorEntry[] | A list of variable mappins if it is a success, or a list of error entries if it is in error |

### VariableMappingEntry

Each variable mapping has the following format:

| Offset | Size   | Name            | Type  | Description                                           |
| ------ | ------ | --------------- | ----- | ----------------------------------------------------- |
| 0      | 1 byte | VariableId      | uint8 | Internal variable id used in the trace variable store |
| 1      | 1 byte | TraceVariableId | uint8 | Variable id used during stream                        |

A different id is used during streaming than during storage, as you can only trace 64 values at a time, but you are able to store up to 256 variables to trace. The host decides what variables need to be traced and thus streamed.

The `VariableMappingEntry` structure is repeated `ArrayLength` times.

### ErrorEntry

Each error entry has the following format:

| Offset | Size   | Name       | Type  | Description                                           |
| ------ | ------ | ---------- | ----- | ----------------------------------------------------- |
| 0      | 1 byte | VariableId | uint8 | Internal variable id used in the trace variable store |
| 1      | 1 byte | Error      | uint8 | The error code for this variable                      |

The `ErrorEntry` structure is repeated `ArrayLength` times.


## TraceData (0x07)

Direction: `Device -> Host`

Contains trace samples generated while tracing is active.
This is a stream of data that is started by the `StartTraceRequest` message and stopped by the `StopTraceRequest` message

### Payload

| Offset | Size    | Name            | Type           | Description                           |
| ------ | ------- | --------------- | -------------- | ------------------------------------- |
| 0      | 1 bytes | TraceDataLength | uint8          | The amount of variables that are send |
| 1      | N bytes | TraceData       | TraceDataEntry | The individual variable traces        |

### TraceDataEntry

Each error entry has the following format:

| Offset | Size    | Name            | Type                   | Description                                           |
| ------ | ------- | --------------- | ---------------------- | ----------------------------------------------------- |
| 0      | 1 byte  | TraceDataHeader | TraceSampleHeaderEntry | Internal variable id used in the trace variable store |
| 1      | N bytes | Value           | uint8[]                | The value in bytes of the trace                       |

The size of `Value` is determined by the `SizeCode` in the `TraceDataHeader`

### TraceSampleHeaderEntry

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

## StopTraceRequest (0x08)

Direction: `Host -> Device`

Stops an active trace session.

### Payload

None.