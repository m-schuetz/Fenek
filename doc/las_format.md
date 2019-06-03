Since I'm currently writing another LAS reader, I'll drop some of the tables in this thread as a semi-persistent place to look them up in the future.

Rearanged the columns because offset reads better if it comes before the size, and the format reads better if it isn't squeezed in between offset and size.

# Header (taken from 1.2 spec)

| Item                                   | Offset    | Size       | Format           | Required |
| -------------                          | -----     | ---        | -------------    | ---      |
| File Signature ("LASF")                | 0         |  4 bytes   | char[4]          |          |
| File Source ID                         | 4         |  2 bytes   | unsigned short   |          |
| Global Encoding                        | 6         |  2 bytes   | unsigned short   | *        |
| Project ID - GUID data 1               | 8         |  4 bytes   | unsigned long    |          |
| Project ID - GUID data 2               | 12        |  2 bytes   | unsigned short   |          |
| Project ID - GUID data 3               | 14        |  2 bytes   | unsigned short   |          |
| Project ID - GUID data 4               | 16        |  8 bytes   | unsigned char[8] |          |
| Version Major                          | 24        |  1 byte    | unsigned char    | *        |
| Version Minor                          | 25        |  1 byte    | unsigned char    | *        |
| System Identifier                      | 26        | 32 bytes   | char[32]         | *        |
| Generating Software                    | 58        | 32 bytes   | char[32]         | *        |
| File Creation Day of Year              | 90        |  2 bytes   | unsigned short   |          |
| File Creation Year                     | 92        |  2 bytes   | unsigned short   |          |
| Header Size                            | 94        |  2 bytes   | unsigned short   | *        |
| Offset to point data                   | 96        |  4 bytes   | unsigned long    | *        |
| Number of Variable Length Records      | 100       |  4 bytes   | unsigned long    | *        |
| Point Data Format ID (0-99 for spec)   | 104       |  1 byte    | unsigned char    | *        |
| Point Data Record Length               | 105       |  2 bytes   | unsigned short   | *        |
| Number of point records                | 107       |  4 bytes   | unsigned long    | *        |
| Number of points by return             | 111       | 20 bytes   | unsigned long[5] | *        |
| X scale factor                         | 131       |  8 bytes   | double           | *        |
| Y scale factor                         | 139       |  8 bytes   | double           | *        |
| Z scale factor                         | 147       |  8 bytes   | double           | *        |
| X offset                               | 155       |  8 bytes   | double           | *        |
| Y offset                               | 163       |  8 bytes   | double           | *        |
| Z offset                               | 171       |  8 bytes   | double           | *        |
| Max X                                  | 179       |  8 bytes   | double           | *        |
| Min X                                  | 187       |  8 bytes   | double           | *        |
| Max Y                                  | 195       |  8 bytes   | double           | *        |
| Min Y                                  | 203       |  8 bytes   | double           | *        |
| Max Z                                  | 211       |  8 bytes   | double           | *        |
| Min Z                                  | 219       |  8 bytes   | double           | *        |
| Total                                  |           |  227 bytes |                  |          |

# Header (taken from 1.4 spec)

| Item                                             | Offset    | Size       | Format                 | Required |
| -------------                                    | -----     | ---        | -------------          | ---      |
| File Signature ("LASF")                          | 0         |  4 bytes   | char[4]                |          |
| File Source ID                                   | 4         |  2 bytes   | unsigned short         |          |
| Global Encoding                                  | 6         |  2 bytes   | unsigned short         | *        |
| Project ID - GUID data 1                         | 8         |  4 bytes   | unsigned long          |          |
| Project ID - GUID data 2                         | 12        |  2 bytes   | unsigned short         |          |
| Project ID - GUID data 3                         | 14        |  2 bytes   | unsigned short         |          |
| Project ID - GUID data 4                         | 16        |  8 bytes   | unsigned char[8]       |          |
| Version Major                                    | 24        |  1 byte    | unsigned char          | *        |
| Version Minor                                    | 25        |  1 byte    | unsigned char          | *        |
| System Identifier                                | 26        | 32 bytes   | char[32]               | *        |
| Generating Software                              | 58        | 32 bytes   | char[32]               | *        |
| File Creation Day of Year                        | 90        |  2 bytes   | unsigned short         |          |
| File Creation Year                               | 92        |  2 bytes   | unsigned short         |          |
| Header Size                                      | 94        |  2 bytes   | unsigned short         | *        |
| Offset to point data                             | 96        |  4 bytes   | unsigned long          | *        |
| Number of Variable Length Records                | 100       |  4 bytes   | unsigned long          | *        |
| Point Data Format ID (0-99 for spec)             | 104       |  1 byte    | unsigned char          | *        |
| Point Data Record Length                         | 105       |  2 bytes   | unsigned short         | *        |
| Legacy Number of point records                          | 107       |  4 bytes   | unsigned long          | *        |
| Legacy Number of points by return                       | 111       | 20 bytes   | unsigned long[5]       | *        |
| X scale factor                                   | 131       |  8 bytes   | double                 | *        |
| Y scale factor                                   | 139       |  8 bytes   | double                 | *        |
| Z scale factor                                   | 147       |  8 bytes   | double                 | *        |
| X offset                                         | 155       |  8 bytes   | double                 | *        |
| Y offset                                         | 163       |  8 bytes   | double                 | *        |
| Z offset                                         | 171       |  8 bytes   | double                 | *        |
| Max X                                            | 179       |  8 bytes   | double                 | *        |
| Min X                                            | 187       |  8 bytes   | double                 | *        |
| Max Y                                            | 195       |  8 bytes   | double                 | *        |
| Min Y                                            | 203       |  8 bytes   | double                 | *        |
| Max Z                                            | 211       |  8 bytes   | double                 | *        |
| Min Z                                            | 219       |  8 bytes   | double                 | *        |
| Start of Waveform Data Packet Record             | 227       |  8 bytes   | unsigned long long     | *        |
| Start of first Extended Variable Length Record   | 235       |  8 bytes   | unsigned long long     | *        |
| Number of Extended Variable Length Records       | 243       |  4 bytes   | unsigned long          | *        |
| Number of point records                          | 247       |  8 bytes   | unsigned long long     | *        |
| Number of points by return                       | 255       |  120 bytes | unsigned long long[15] | *        |
| Total                                            |           |  375 bytes |                        |          |


# Format 0 (taken from 1.4 spec)

| Item                                      | Offset | Size        | Format                | Required |
| -------------                             | -----  | ---         | ------------          | ---      |
| X                                         | 0      |  4 bytes    | long                  | *        |
| Y                                         | 4      |  4 bytes    | long                  | *        |
| Z                                         | 8      |  4 bytes    | long                  | *        |
| Intensity                                 | 12     |  2 bytes    | unsigned short        |          |
| Return Number                             | 14     |  3 bits     | 3 bits (bits 0, 1, 2) | *        |
| Number of Returns                         |        |  3 bits     | 3 bits (bits 3, 4, 5) | *        |
| Scan Direction Flag                       |        |  1 bit      | 1 bit (bit 6)         | *        |
| Edge of Flight Line                       |        |  1 bit      | 1 bit (bit 7)         | *        |
| Classification                            | 15     |  1 bytes    | unsigned char         | *        |
| Scan Angle Rank (-90 to +90) - Left Side  | 16     |  1 bytes    | unsigned char         | *        |
| User Data                                 | 17     |  1 bytes    | unsigned char         |          |
| Point Source ID                           | 18     |  2 bytes    | unsigned short        | *        |
| Total                                     |        |  20 bytes   |                       |          |

# Format 1 (taken from 1.4 spec)

| Item                                      | Offset | Size        | Format                | Required |
| -------------                             | -----  | ---         | ------------          | ---      |
| X                                         | 0      |  4 bytes    | long                  | *        |
| Y                                         | 4      |  4 bytes    | long                  | *        |
| Z                                         | 8      |  4 bytes    | long                  | *        |
| Intensity                                 | 12     |  2 bytes    | unsigned short        |          |
| Return Number                             | 14     |  3 bits     | 3 bits (bits 0, 1, 2) | *        |
| Number of Returns                         |        |  3 bits     | 3 bits (bits 3, 4, 5) | *        |
| Scan Direction Flag                       |        |  1 bit      | 1 bit (bit 6)         | *        |
| Edge of Flight Line                       |        |  1 bit      | 1 bit (bit 7)         | *        |
| Classification                            | 15     |  1 bytes    | unsigned char         | *        |
| Scan Angle Rank (-90 to +90) - Left Side  | 16     |  1 bytes    | unsigned char         | *        |
| User Data                                 | 17     |  1 bytes    | unsigned char         |          |
| Point Source ID                           | 18     |  2 bytes    | unsigned short        | *        |
| GPS Time                                  | 20     |  8 bytes    | double                | *        |
| Total                                     |        |  28 bytes   |                       |          |

# Format 2 (taken from 1.2 spec)

| Item                                      | Offset | Size        | Format                | Required |
| -------------                             | -----  | ---         | ------------          | ---      |
| X                                         | 0      |  4 bytes    | long                  | *        |
| Y                                         | 4      |  4 bytes    | long                  | *        |
| Z                                         | 8      |  4 bytes    | long                  | *        |
| Intensity                                 | 12     |  2 bytes    | unsigned short        |          |
| Return Number                             | 14     |  3 bits     | 3 bits (bits 0, 1, 2) | *        |
| Number of Returns                         |        |  3 bits     | 3 bits (bits 3, 4, 5) | *        |
| Scan Direction Flag                       |        |  1 bit      | 1 bit (bit 6)         | *        |
| Edge of Flight Line                       |        |  1 bit      | 1 bit (bit 7)         | *        |
| Classification                            | 15     |  1 bytes    | unsigned char         | *        |
| Scan Angle Rank (-90 to +90) - Left Side  | 16     |  1 bytes    | unsigned char         | *        |
| User Data                                 | 17     |  1 bytes    | unsigned char         |          |
| Point Source ID                           | 18     |  2 bytes    | unsigned short        | *        |
| Red                                       | 20     |  2 bytes    | unsigned short        | *        |
| Green                                     | 22     |  2 bytes    | unsigned short        | *        |
| Blue                                      | 24     |  2 bytes    | unsigned short        | *        |
| Total                                     |        |  26 bytes   |                       |          |

# Format 3 (taken from 1.4 spec)

| Item                                      | Offset | Size        | Format                | Required |
| -------------                             | -----  | ---         | ------------          | ---      |
| X                                         | 0      |  4 bytes    | long                  | *        |
| Y                                         | 4      |  4 bytes    | long                  | *        |
| Z                                         | 8      |  4 bytes    | long                  | *        |
| Intensity                                 | 12     |  2 bytes    | unsigned short        |          |
| Return Number                             | 14     |  3 bits     | 3 bits (bits 0, 1, 2) | *        |
| Number of Returns                         |        |  3 bits     | 3 bits (bits 3, 4, 5) | *        |
| Scan Direction Flag                       |        |  1 bit      | 1 bit (bit 6)         | *        |
| Edge of Flight Line                       |        |  1 bit      | 1 bit (bit 7)         | *        |
| Classification                            | 15     |  1 bytes    | unsigned char         | *        |
| Scan Angle Rank (-90 to +90) - Left Side  | 16     |  1 bytes    | unsigned char         | *        |
| User Data                                 | 17     |  1 bytes    | unsigned char         |          |
| Point Source ID                           | 18     |  2 bytes    | unsigned short        | *        |
| GPS Time                                  | 20     |  8 bytes    | double                | *        |
| Red                                       | 28     |  2 bytes    | unsigned short        | *        |
| Green                                     | 30     |  2 bytes    | unsigned short        | *        |
| Blue                                      | 32     |  2 bytes    | unsigned short        | *        |
| Total                                     |        |  34 bytes   |                       |          |

# Format 4 (taken from 1.4 spec)

| Item                                      | Offset | Size        | Format                | Required |
| -------------                             | -----  | ---         | ------------          | ---      |
| X                                         | 0      |  4 bytes    | long                  | *        |
| Y                                         | 4      |  4 bytes    | long                  | *        |
| Z                                         | 8      |  4 bytes    | long                  | *        |
| Intensity                                 | 12     |  2 bytes    | unsigned short        |          |
| Return Number                             | 14     |  3 bits     | 3 bits (bits 0, 1, 2) | *        |
| Number of Returns                         |        |  3 bits     | 3 bits (bits 3, 4, 5) | *        |
| Scan Direction Flag                       |        |  1 bit      | 1 bit (bit 6)         | *        |
| Edge of Flight Line                       |        |  1 bit      | 1 bit (bit 7)         | *        |
| Classification                            | 15     |  1 bytes    | unsigned char         | *        |
| Scan Angle Rank (-90 to +90) - Left Side  | 16     |  1 bytes    | unsigned char         | *        |
| User Data                                 | 17     |  1 bytes    | unsigned char         |          |
| Point Source ID                           | 18     |  2 bytes    | unsigned short        | *        |
| GPS Time                                  | 20     |  8 bytes    | double                | *        |
| Wave Packet Descriptor Index              | 28     |  1 bytes    | unsigned char         | *        |
| Byte offset to waveform data              | 29     |  8 bytes    | unsigned long long    | *        |
| Waveform packet size in bytes             | 37     |  4 bytes    | unsigned long         | *        |
| Return Point Waveform Location            | 41     |  4 bytes    | float                 | *        |
| X(t)                                      | 45     |  4 bytes    | float                 | *        |
| Y(t)                                      | 49     |  4 bytes    | float                 | *        |
| Z(t)                                      | 53     |  4 bytes    | float                 | *        |
| Total                                     |        |  57 bytes   |                       |          |

# Format 5 (taken from 1.4 spec)

| Item                                      | Offset | Size        | Format                | Required |
| -------------                             | -----  | ---         | ------------          | ---      |
| X                                         | 0      |  4 bytes    | long                  | *        |
| Y                                         | 4      |  4 bytes    | long                  | *        |
| Z                                         | 8      |  4 bytes    | long                  | *        |
| Intensity                                 | 12     |  2 bytes    | unsigned short        |          |
| Return Number                             | 14     |  3 bits     | 3 bits (bits 0, 1, 2) | *        |
| Number of Returns                         |        |  3 bits     | 3 bits (bits 3, 4, 5) | *        |
| Scan Direction Flag                       |        |  1 bit      | 1 bit (bit 6)         | *        |
| Edge of Flight Line                       |        |  1 bit      | 1 bit (bit 7)         | *        |
| Classification                            | 15     |  1 bytes    | unsigned char         | *        |
| Scan Angle Rank (-90 to +90) - Left Side  | 16     |  1 bytes    | unsigned char         | *        |
| User Data                                 | 17     |  1 bytes    | unsigned char         |          |
| Point Source ID                           | 18     |  2 bytes    | unsigned short        | *        |
| GPS Time                                  | 20     |  8 bytes    | double                | *        |
| Red                                       | 28     |  2 bytes    | unsigned short        | *        |
| Green                                     | 30     |  2 bytes    | unsigned short        | *        |
| Blue                                      | 32     |  2 bytes    | unsigned short        | *        |
| Wave Packet Descriptor Index              | 34     |  1 bytes    | unsigned char         | *        |
| Byte offset to waveform data              | 35     |  8 bytes    | unsigned long long    | *        |
| Waveform packet size in bytes             | 43     |  4 bytes    | unsigned long         | *        |
| Return Point Waveform Location            | 47     |  4 bytes    | float                 | *        |
| X(t)                                      | 51     |  4 bytes    | float                 | *        |
| Y(t)                                      | 55     |  4 bytes    | float                 | *        |
| Z(t)                                      | 59     |  4 bytes    | float                 | *        |
| Total                                     |        |  63 bytes   |                       |          |

# Format 6 (taken from 1.4 spec)

| Item                                      | Offset | Size        | Format                | Required |
| -------------                             | -----  | ---         | ------------          | ---      |
| X                                         | 0      |  4 bytes    | long                  | *        |
| Y                                         | 4      |  4 bytes    | long                  | *        |
| Z                                         | 8      |  4 bytes    | long                  | *        |
| Intensity                                 | 12     |  2 bytes    | unsigned short        |          |
| Return Number                             | 14     |  4 bits     | 4 bits (bits 0 - 3)   | *        |
| Number of Returns (given Pulse)           |        |  4 bits     | 4 bits (bits 4 - 7)   | *        |
| Classification Flags                      | 15     |  4 bits     | 4 bits (bits 0 - 3)   |          |
| Scanner Channel                           |        |  2 bits     | 2 bits (bits 4 - 5)   | *        |
| Scan Direction Flag                       |        |  1 bit      | 1 bit (bit 6)         | *        |
| Edge of Flight Line                       |        |  1 bit      | 1 bit (bit 7)         | *        |
| Classification                            | 16     |  1 bytes    | unsigned char         | *        |
| User Data                                 | 17     |  1 bytes    | unsigned char         |          |
| Scan Angle                                | 18     |  2 bytes    | short                 | *        |
| Point Source ID                           | 20     |  2 bytes    | unsigned short        | *        |
| GPS Time                                  | 22     |  8 bytes    | double                | *        |
| Total                                     |        |  30 bytes   |                       |          |


# Format 7 (taken from 1.4 spec)

| Item                                      | Offset | Size        | Format                | Required |
| -------------                             | -----  | ---         | ------------          | ---      |
| X                                         | 0      |  4 bytes    | long                  | *        |
| Y                                         | 4      |  4 bytes    | long                  | *        |
| Z                                         | 8      |  4 bytes    | long                  | *        |
| Intensity                                 | 12     |  2 bytes    | unsigned short        |          |
| Return Number                             | 14     |  4 bits     | 4 bits (bits 0 - 3)   | *        |
| Number of Returns (given Pulse)           |        |  4 bits     | 4 bits (bits 4 - 7)   | *        |
| Classification Flags                      | 15     |  4 bits     | 4 bits (bits 0 - 3)   |          |
| Scanner Channel                           |        |  2 bits     | 2 bits (bits 4 - 5)   | *        |
| Scan Direction Flag                       |        |  1 bit      | 1 bit (bit 6)         | *        |
| Edge of Flight Line                       |        |  1 bit      | 1 bit (bit 7)         | *        |
| Classification                            | 16     |  1 bytes    | unsigned char         | *        |
| User Data                                 | 17     |  1 bytes    | unsigned char         |          |
| Scan Angle                                | 18     |  2 bytes    | short                 | *        |
| Point Source ID                           | 20     |  2 bytes    | unsigned short        | *        |
| GPS Time                                  | 22     |  8 bytes    | double                | *        |
| Red                                       | 30     |  2 bytes    | unsigned short        | *        |
| Green                                     | 32     |  2 bytes    | unsigned short        | *        |
| Blue                                      | 34     |  2 bytes    | unsigned short        | *        |
| Total                                     |        |  36 bytes   |                       |          |