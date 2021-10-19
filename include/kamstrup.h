void kamSend(byte const *msg, int msgsize);
unsigned short kamReceive(byte recvmsg[]);
float kamDecode(unsigned short const kreg, byte const *msg);
long crc_1021(byte const *inmsg, unsigned int len);
unsigned short kamReceive(byte recvmsg[]);