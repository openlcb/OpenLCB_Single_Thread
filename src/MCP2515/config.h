#ifndef	CONFIG_H
#define	CONFIG_H

// -----------------------------------------------------------------------------
/* Global settings for building the can-lib and application program.
 *
 * The following two #defines must be set identically for the can-lib and
 * your application program. They control the underlying CAN struct. If the
 * settings disagree, the underlying CAN struct will be broken, with
 * unpredictable results.
 * If can.h detects that any of the #defines is not defined, it will set them
 * to the default values shown here, so it is in your own interest to have a
 * consistent setting. Ommiting the #defines in both can-lib and application
 * program will apply the defaults in a consistent way too.
 *
 * Select if you want to use 29 bit identifiers.
 */
#define	SUPPORT_EXTENDED_CANID	1

/* Select if you want to use timestamps.
 * Timestamps are sourced from a register internal to the AT90CAN.
 * Selecting them on any other controller will have no effect, they will
 * be 0 all the time.
 */
#define	SUPPORT_TIMESTAMPS		0


#define	SUPPORT_MCP2515			1

// -----------------------------------------------------------------------------
/* Setting for MCP2515
 *
 * Declare which pins you are using for communication.
 * Remember NOT to use them in your application!
 * It is a good idea to use bits from the port that carries MOSI, MISO, SCK.
 */
#define	MCP2515_CS				B,2   // B,2 for LEDuino; B,1 for Railroad Shield
#define	MCP2515_INT				D,3

#endif	// CONFIG_H
