#ifndef NodeID_h
#define NodeID_h

#include <string.h>
#include <stdint.h>

#define NODEID_SIZE	6

class NodeID {
 public:
    uint8_t  val[NODEID_SIZE];
  
  NodeID() {
    memset(val, 0, sizeof(val));
  }
  
  NodeID(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5) {
      val[0] = b0;
      val[1] = b1;
      val[2] = b2;
      val[3] = b3;
      val[4] = b4;
      val[5] = b5;
  }
    
  bool equals(NodeID* n) {
    return memcmp(val, n->val, sizeof(val)) == 0;
  }
};

#endif
