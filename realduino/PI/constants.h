#ifndef __CONSTANTS_INC__
#define __CONSTANTS_INC__

/* 
 *  This file containts all the packet types, commands
 *  and status constants
 *  
 */

// Packet types
typedef enum
{
  PACKET_TYPE_COMMAND = 0,
  PACKET_TYPE_RESPONSE = 1,
  PACKET_TYPE_ERROR = 2,
  PACKET_TYPE_MESSAGE = 3,
  PACKET_TYPE_HELLO = 4
} TPacketType;

// Response types. This goes into the command field
typedef enum
{
  RESP_OK = 0,
  RESP_STATUS=1,
  RESP_BAD_PACKET = 2,
  RESP_BAD_CHECKSUM = 3,
  RESP_BAD_COMMAND = 4,
  RESP_BAD_RESPONSE = 5, 
  RESP_COLOUR = 6
} TResponseType;


// Commands
typedef enum
{
  COMMAND_STOP = 4,
  COMMAND_DETECT_COLOUR = 7,
  COMMAND_F = 8,
  COMMAND_B = 9,
  COMMAND_L = 10,
  COMMAND_R = 11,
  COMMAND_INCREASE = 12,
  COMMAND_DECREASE = 13
} TCommandType;

typedef enum
{
  STOP = 0,
  FORWARD = 1,
  BACKWARD = 2,
  LEFT = 3,
  RIGHT = 4
} TDirection;

volatile TDirection dir = STOP;
#endif
