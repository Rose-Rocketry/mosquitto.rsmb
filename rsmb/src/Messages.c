/*******************************************************************************
 * Copyright (c) 2007, 2013 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution. 
 *
 * The Eclipse Public License is available at 
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at 
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial API and implementation and/or initial documentation
 *******************************************************************************/

/**
 * @file
 * Message retrieval from storage and indexing
 */

#include "Messages.h"
#include "Log.h"
#include "StackTrace.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

#if defined(WIN32)
#define snprintf _snprintf
#endif

#include "Heap.h"


#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))


static char* protocol_message_list[] =
{
	"%d %s -> CONNECT cleansession: %d noLocal: %d (%d)", /* 0, was 131, 68 and 69 */
	"%d %s <- CONNACK rc: %d", /* 1, was 132 */
	"%d %s -> CONNACK rc: %d (%d)", /* 2, was 138 */
	"%d %s <- PINGREQ", /* 3, was 35 */
	"%d %s -> PINGRESP (%d)", /* 4 */
	"%d %s <- DISCONNECT", /* 5 */
	"%d %s <- SUBSCRIBE msgid: %d", /* 6, was 39 */
	"%d %s -> SUBACK msgid: %d (%d)", /* 7, was 40 */
	"%d %s <- UNSUBSCRIBE msgid: %d", /* 8, was 41 */
	"%d %s -> UNSUBACK msgid: %d (%d)", /* 9 */
	"%d %s -> PUBLISH msgid: %d qos: %d retained: %d (%d)", /* 10, was 42 */
	"%d %s <- PUBLISH msgid: %d qos: %d retained: %d", /* 11, was 46 */
	"%d %s -> PUBACK msgid: %d (%d)", /* 12, was 47 */
	"%d %s -> PUBREC msgid: %d (%d)", /* 13, was 48 */
	"%d %s <- PUBACK msgid: %d", /* 14, was 49 */
	"%d %s <- PUBREC msgid: %d", /* 15, was 53 */
	"%d %s -> PUBREL msgid: %d (%d)", /* 16, was 57 */
	"%d %s <- PUBREL msgid %d", /* 17, was 58 */
	"%d %s -> PUBCOMP msgid %d (%d)", /* 18, was 62 */
	"%d %s <- PUBCOMP msgid:%d", /* 19, was 63 */
	"%d %s -> PINGREQ (%d)", /* 20, was 137 */
	"%d %s <- PINGRESP", /* 21, was 70 */
	"%d %s -> SUBSCRIBE msgid: %d (%d)", /* 22, was 72 */
	"%d %s <- SUBACK msgid: %d", /* 23, was 73 */
	"%d %s <- UNSUBACK msgid: %d", /* 24, was 74 */
	"%d %s -> UNSUBSCRIBE msgid: %d (%d)", /* 25, was 106 */
	"%d %s <- CONNECT", /* 26 */
	"%d %s -> PUBLISH qos: 0 retained: %d (%d)", /* 27 */
	"%d %s -> DISCONNECT (%d)", /* 28 */
	"%d %s -- reserved", /* 29 */
#if defined(MQTTS)
	"%d %s %s -> MQTT-S ADVERTISE gateway_id: %d duration: %d (%d)", /* 30 */
	"%d %s %s <- MQTT-S ADVERTISE gateway_id: %d duration: %d", /* 31 */
    "%d %s %s -> MQTT-S SEARCHGW", /* 32 */
    "%d %s %s <- MQTT-S SEARCHGW", /* 33 */
    "%d %s %s -> MQTT-S GWINFO", /* 34 */
    "%d %s %s <- MQTT-S GWINFO", /* 35 */
    "reserved", /* 36 */
    "reserved", /* 37 */
    "%d %s %s -> MQTT-S CONNECT cleansession: %d (%d)", /* 38 */
    "%d %s %s <- MQTT-S CONNECT cleansession: %d", /* 39 */
    "%d %s %s -> MQTT-S CONNACK returncode %d (%d)", /* 40 */
    "%d %s %s <- MQTT-S CONNACK returncode %d", /* 41 */
    "%d %s %s -> MQTT-S WILLTOPICREQ (%d)", /* 42 */
    "%d %s %s <- MQTT-S WILLTOPICREQ", /* 43 */
    "%d %s %s -> MQTT-S WILLTOPIC qos: %d retained: %d: topicname %.10s (%d)", /* 44 */
    "%d %s %s <- MQTT-S WILLTOPIC qos: %d retained: %d: topicname %.10s", /* 45 */
    "%d %s %s -> MQTT-S WILLMSGREQ (%d)", /* 46 */
    "%d %s %s <- MQTT-S WILLMSGREQ", /* 47 */
    "%d %s %s -> MQTT-S WILLMSG msg: %.20s (%d)", /* 48 */
    "%d %s %s <- MQTT-S WILLMSG msg: %.20s", /* 49 */
    "%d %s %s -> MQTT-S REGISTER msgid: %d topicid: %d topicname: %.10s (%d)", /* 50 */
    "%d %s %s <- MQTT-S REGISTER msgid: %d topicid: %d topicname: %.10s", /* 51 */
    "%d %s %s -> MQTT-S REGACK msgid: %d topicid: %d returncode: %d (%d)", /* 52 */
    "%d %s %s <- MQTT-S REGACK msgid: %d topicid: %d returncode: %d", /* 53 */
	"%d %s %s -> MQTT-S PUBLISH msgid: %d qos: %d retained: %d (%d)", /* 54 */
	"%d %s %s <- MQTT-S PUBLISH msgid: %d qos: %d retained: %d", /* 55 */
	"%d %s %s -> MQTT-S PUBACK msgid: %d (%d)", /* 56 */
	"%d %s %s <- MQTT-S PUBACK msgid: %d", /* 57 */
	"%d %s %s -> MQTT-S PUBCOMP msgid: %d (%d)", /* 58 */
	"%d %s %s <- MQTT-S PUBCOMP msgid: %d", /* 59 */
	"%d %s %s -> MQTT-S PUBREC msgid: %d (%d)", /* 60 */
	"%d %s %s <- MQTT-S PUBREC msgid: %d", /* 61 */
	"%d %s %s -> MQTT-S PUBREL msgid: %d (%d)", /* 62 */
	"%d %s %s <- MQTT-S PUBREL msgid: %d", /* 63 */
    "reserved", /* 64 */
    "reserved", /* 65 */
	"%d %s %s -> MQTT-S SUBSCRIBE msgid: %d qos: %d topicIdType %d", /* 66 */
	"%d %s %s <- MQTT-S SUBSCRIBE msgid: %d qos: %d topicIdType %d", /* 67 */
	"%d %s %s -> MQTT-S SUBACK msgid: %d topicid: %d returncode: %d (%d)", /* 68 */
	"%d %s %s <- MQTT-S SUBACK msgid: %d topicid: %d returncode: %d", /* 69 */
	"%d %s %s -> MQTT-S UNSUBSCRIBE msgid: %d qos: %d topicIdType %d", /* 70 */
	"%d %s %s <- MQTT-S UNSUBSCRIBE msgid: %d qos: %d topicIdType %d", /* 71 */
	"%d %s %s -> MQTT-S UNSUBACK msgid: %d (%d)", /* 72 */
	"%d %s %s <- MQTT-S UNSUBACK msgid: %d", /* 73 */
	"%d %s %s -> MQTT-S PINGREQ (%d)", /* 74 */
	"%d %s %s <- MQTT-S PINGREQ", /* 75 */
	"%d %s %s -> MQTT-S PINGRESP (%d)", /* 76 */
	"%d %s %s <- MQTT-S PINGRESP", /* 77 */
	"%d %s %s -> MQTT-S DISCONNECT duration: %d (%d)", /* 78 */
	"%d %s %s <- MQTT-S DISCONNECT duration: %d", /* 79 */
    "reserved", /* 80 */
    "reserved", /* 81 */
	"%d %s %s -> MQTT-S WILLTOPICUPD msgid: %d (%d)", /* 82 */
	"%d %s %s <- MQTT-S WILLTOPICUPD msgid: %d", /* 83 */
	"%d %s %s -> MQTT-S WILLTOPICRESP returnCode: %d (%d)", /* 84 */
	"%d %s %s <- MQTT-S WILLTOPICRESP returnCode: %d", /* 85 */
	"%d %s %s -> MQTT-S WILLMSGUPD message: %.10s (%d)", /* 86 */
	"%d %s %s <- MQTT-S WILLMSGUPD message: %.10s", /* 87 */
	"%d %s %s -> MQTT-S WILLMSGRESP returnCode: %d (%d)", /* 88 */
	"%d %s %s <- MQTT-S WILLMSGRESP returnCode: %d", /* 89 */
#endif
};

static char* trace_message_list[] =
{
	"Processing queued messages for client %s", /* 0, was 25 */
	"Moving message from queued to inflight for client %s", /* 1, was 26 */
	"Removed client %s from bstate->clients, socket %d", /* 2, was 37 */
	"Queueing publish to client %s at qos %d", /* 3, was 44 */
	"PUBACK received from client %s for message id %d - removing publication", /* 4, was 52 */
	"PUBCOMP received from client %s for message id %d - removing publication", /* 5, was 67 */
	"FD_SETSIZE is %d", /* 6, was 76 */
	"We already have a socket %d in the list", /* 7, was 81 */
	"Return code %d from read select", /* 8, was 83 */
	"Return code %d from write select", /* 9, was 84 */
	"Accepted socket %d from %s:%d", /* 10, was 85 */
	"GetReadySocket returning %d", /* 11, was 86 */
	"%d bytes expected but %d bytes now received", /* 12, was 87 */
	"Removed socket %d", /* 13, was 90 */
	"New socket %d for %s, port %d", /* 14, was 93 */
	"Connect pending", /* 15, was 94 */
	"ContinueWrite wrote +%lu bytes on socket %d", /* 16, was 95 */
	"Packet_Factory: unhandled packet type %d", /* 17, was 107 */
	"will %s %s %d", /* 18, was 108 */
	"index is now %d, headerlen %d", /* 19, was 110 */
	"queueChar: index is now %d, headerlen %d", /* 20, was 114 */
	"Updating subscription %s, %s, %d", /* 21, was 115 */
	"Adding subscription %s, %s, %d", /* 22, was 116 */
	"Removing subscription %s, %s, %d", /* 23, was 117 */
	"Subscription %s %d %s", /* 24, was 119 */
	"Adding client %s to subscribers list", /* 25, was 120 */
	"Matching %s against %s", /* 26, was 121 */
	"Matched %s against %s", /* 27, was 122 */
	"%s connected %d, connect_state %d", /* 28, was 126 */
	"%*s(%d)> %s:%d", /* 29 */
	"%*s(%d)< %s:%d", /* 30 */
	"%*s(%d)< %s:%d (%d)", /* 31 */
	"No bytes written in publication, so no need to suspend write", /* 32 */
	"Partial write: %ld bytes of %d actually written on socket %d", /* 33 */
	"Failed to remove socket %d", /* 34 */
	"Failed to remove pending write from socket buffer list",  /* 35 */
	"Failed to remove pending write from list",  /* 36 */
	"Storing unsent QoS 0 message", /* 37 */
	"Unable to remove message from queued list", /* 38 */
	"Failed to remove client from bstate->clients", /* 39 */
};

static char* message_list[] = {
	"Could not read configuration file %s", /* 0 */
	"No value for keyword %s on line %d", /* 1 */
	"Unrecognized topic direction %s", /* 2 */
	"Setting property \"%s\" to %s", /* 3 */
	"Unrecognized boolean value %s on line number %d", /* 4 */
	"Setting property \"%s\" to \"%s\"", /* 5 */
	"Adding value \"%s\" to list \"%s\"", /* 6 */
	"Setting property \"%s\" to %d", /* 7 */
	"Unrecognized configuration keyword %s on line number %d", /* 8 */
	"Cannot open %s file %s for writing, %ss will not be saved", /* 9 */
	"Cannot open %s file %s for reading, %ss will not be restored", /* 10 */
	"Restoring %ss from file %s", /* 11 */
	"Wildcard in publish topic %.20s from client %s not allowed", /* 12 */
	"Internal error; FFDC written to file %s", /* 13 */
	"MQTT protocol starting, listening on port %d", /* 14 */
	"Cannot start listener on port %d", /* 15 */
	"MQTT protocol stopping", /* 16 */
	"Closing client %s", /* 17 */
	"Socket error for client identifier %s, socket %d, peer address %s; ending connection", /* 18 */
	"Bad packet for client identifier %s, socket %d, peer address %s; ending connection", /* 19 */
	"Socket error on socket %d, peer address %s; ending connection", /* 20 */
	"Badly formed packet on socket %d, peer address %s; ending connection", /* 21 */
	"Unknown MQTT packet type %d on socket %d", /* 22 */
	"Connect was not first packet on socket %d, peer address %s; got %s", /* 23 */
	"%d second keepalive timeout for client %s, ending connection", /* 24 */
	"Incorrect configuration: acl_file requires password_file to be specified", /* 25 */
	NULL,
	NULL,
	"Trying PUBLISH again for client %s, socket %d, message identifier %d", /* 28 */
	"Socket error for client %s, socket %d, ending connection", /* 29 */
	"Trying PUBREL again for client %s, message identifier %d", /* 30 */
	"Refusing connection attempt for unauthorized client identifier %s", /* 31 */
	"Connection attempt using unsupported protocol %s version %d received", /* 32 */
	"Connection attempt to listener %d received from client %s on address %s", /* 33 */
	"Duplicate connection attempt received for client identifier \"%s\" from address %s, ending oldest connection", /* 34 */
	NULL,
	NULL,
	NULL,
	"Disconnection request received from client %s", /* 38 */
	"Invalid user entry on line number %d", /* 39 */
	"Unrecognized user %s on line number %d", /* 40 */
	"Invalid access control rule on line number %d", /* 41 */
	"Uptime: %d seconds", /* 42 */
	"Messages received: %d", /* 43 */
	"Messages sent: %d", /* 44 */
	"Queued message limit reached for client %s. Number of messages discarded so far: %d", /* 45 */
	"Broker stopping", /* 46 */
	"Broker stopped", /* 47 */
	"First Failure Data Capture (FFDC) condition occurred, but FFDC writing turned off", /* 48 */
	"Configuration file name is %s", /* 49 */
	"Packet %s received from client %s for message identifier %d, but no record of that message identifier found", /* 50 */
	"Packet %s received from client %s for message identifier %d, but message is wrong QoS, %d", /* 51 */
	"Packet %s received from client %s for message identifier %d, but message is in wrong state", /* 52 */
	"Version %s, %s", /* 53 */
	"Features included:", /* 54 */
	"Maximum heap use: %d bytes", /* 55 */
	"Bridge connection %s not started because its client identifier %s is a duplicate", /* 56 */
	"Connection %s is deleted", /* 57 */
	"Error deleting connection %s", /* 58 */
	"No bridge connection with name %s", /* 59 */
	"Stopping bridge connection %s", /* 60 */
	"Unable to stop connection %s", /* 61 */
	"Connection %s is now stopped", /* 62 */
	"Stopping connection %s on idle timeout", /* 63 */
	"%d queued messages discarded for client %s", /* 64 */
	"Unable to clear retained flag for system topic %s", /* 65 */
	"Discarding retained message with invalid topic name %s", /* 66 */
	"Error getting network format for address %s", /* 67 */
	"Processing command file %s", /* 68 */
	NULL,
	NULL,
	"Unexpected ping response received for client %s", /* 71 */
	NULL,
	NULL,
	NULL,
	"Socket error %d (%s) in %s for socket %d", /* 75 */
	NULL,
	"Cannot open socket", /* 77 */
	"Cannot bind port %d", /* 78 */
	"TCP listen call failed for port %d", /* 79 */
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"%s is not a valid IP address", /* 92 */
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"Trying bridge connection %s address %s again, without private protocol", /* 99 */
	"Autosaving persistent data after %d changes", /* 100 */
	"Autosaving persistent data after %d second interval", /* 101 */
	NULL,
	NULL,
	"Saving persistence state at user request", /* 104 */
	"Ignoring user request to save state because there are no changes", /* 105 */
	NULL,
	NULL,
	NULL,
	"Failed to setsockopt SO_REUSEADDR on listening port %d", /* 109 */
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"Trying backup bridge connection %s", /* 123 */
	"Starting bridge connection %s", /* 124 */
	"Error starting bridge connection", /* 125 */
	NULL,
	"Starting reconnection attempt for bridge connection %s, address %s", /* 127 */
	"TCP connect timeout for bridge connection %s", /* 128 */
	"Can't get connection completion state from socket", /* 129 */
	"Connect for bridge client %s, address %s, failed with TCP error code %d", /* 130 */
	NULL,
	"Connection acknowledgement rc %d received for client %s", /* 132 */
	"Bridge connection %s to %s now established", /* 133 */
	"Closing bridge backup connection %s", /* 134 */
	"Inbound bridge topic %s does not match any pattern for connection %s", /* 135 */
	"Outbound bridge topic %s does not match any pattern for connection %s", /* 136 */
	NULL,
	NULL,
	"retained message", /* 139 */
	"subscription", /* 140 */
	"Refusing connection attempt by client %s; maximum number of connections %d already reached for listener %d", /* 141 */
	"Incorrect configuration: no addresses for bridge connection %s", /* 142 */
	"Ping response not received within %d seconds for bridge connection %s; ending connection", /* 143 */
	"Connection with name %s already exists; add failed", /* 144 */
	"Message queue for client %s is more than %d%% full", /* 145 */
	"Message queue for client %s is less than %d%% full", /* 146 */
	"Error saving retained message persistence file", /* 147 */
	"Error saving subscription persistence file", /* 148 */
	"Client %s is not authorized to publish to topic: %s", /* 149 */
	"Client %s is not authorized to subscribe to topic: %s", /* 150 */
	"Cannot give read access to topic: %s", /* 151 */
	"Unrecognized configuration value %s on line number %d", /* 152 */
	"Invalid topic syntax in subscription %.20s from client identifier %s, peer address %s", /* 153 */
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"MQTT-S protocol starting, listening on port %d", /* 300 */
	"MQTT-S protocol stopping", /* 301 */
	"Unknown interface %s for if_nametoindex", /* 302 */
	"Adding multicast interface %s on interface %s" /* 303 */
};

/**
 * Get a log message by its index
 * @param index the integer index
 * @param log_level the log level, used to determine which message list to use
 * @return the message format string
 */
char* Messages_get(int index, int log_level)
{
	char* msg = NULL;

	if (log_level < TRACE_PROTOCOL || log_level > LOG_WARNING)
		msg = (index >= 0 && index < ARRAY_SIZE(trace_message_list)) ? trace_message_list[index] : NULL;
	else if (log_level == TRACE_PROTOCOL)
		msg = (index >= 0 && index < ARRAY_SIZE(protocol_message_list)) ? protocol_message_list[index] : NULL;
	else
		msg = (index >= 0 && index < ARRAY_SIZE(message_list)) ? message_list[index] : NULL;
	return msg;
}
