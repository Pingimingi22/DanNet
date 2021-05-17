#pragma once

/// ============================================= DanNet Header ============================================= //
//	The idea behind this header file is that it will contain everything user's of DanNet will need in their
//  programs. This prevent the need to #include a bunch of different headers from DanNet and will allow for one
//  simple catch-all header file.
//  ========================================================================================================= //

#include "MessageIdentifiers.h"									// =================== NOTE =================== //
#include "CorePackets.h"										// The order in which these includes are called	//
#include "Peer.h"												// matters. An example is Packet.h, it requires	//
#include "Packet.h"												// that Peer.h be included before it since it	//
#include "PacketPriorities.h"									// relies on headers in Peer.h					//
																// ============================================ //
