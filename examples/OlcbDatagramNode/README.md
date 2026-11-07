**OlcbDatagramNode**

Two boards run this sketch. Give each a different `NODE_ADDRESS` from a range you control. See http://registry.openlcb.org/uniqueidranges

`NUM_EVENT` is 0 and `eidtab` is empty. The CDI group **Datagram peer** holds one event. Its description says the first six bytes are the other node's Node ID, and that the event is not produced or consumed. In JMRI, write an event whose first six bytes are the other board's Node ID. The last two bytes are ignored.

`RESET_TO_FACTORY_DEFAULTS` starts at 1. Upload once, set it back to 0, and upload again. The first upload clears EEPROM and writes a zero peer event. The second upload leaves EEPROM alone, so later JMRI writes survive.

Until the peer event is non-zero, `send()` does nothing. After both peers are set, each board sends a ping every two seconds: content id `0xF0`, payload `0x01`. The other board answers once with payload `0x02` and does not answer a reply. `LED_BUILTIN` toggles when a datagram is accepted, on boards that have that pin.

Changing the peer event in JMRI redirects the next send. A CDI read still completes while the pings are running, because this client uses the node's one Datagram transport and does not construct a second `Datagram`.

`NodeDatagram.h` is included once, after `OpenLCBMid.h`. `loop()` calls `Olcb_process()`, then `NodeDatagram::check()`.
