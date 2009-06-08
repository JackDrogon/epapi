/**
 * @file epapi_pkt.cc
 *
 * @date   2009-06-06
 * @author Jean-Lou Dupont
 */
#include "epapi.h"


// =========================================
// Pkt Class
// =========================================

/**
 * Creates RX packet type
 */
Pkt::Pkt() {
	last_error=0;
	sz=0;
	buf=NULL;
}//

Pkt::~Pkt() {

	if (NULL!=buf)
		free(buf);

	if (NULL!=tbuf)
		ei_x_free(tbuf);

}//

ei_x_buff *
Pkt::getTxBuf(void) {

	if (NULL==tbuf) {
		tbuf = (ei_x_buff *) malloc(sizeof(ei_x_buff));
	}

	if (NULL==tbuf) {
		last_error = EEPAPI_MALLOC;
		return NULL;
	}

	return tbuf;
}

unsigned char *
Pkt::getBuf(void) {

	if (NULL!=buf)
		return (unsigned char *)buf;

	if (0==sz) {
		buf = (char *) malloc(Pkt::DSZ);
	}
	if (NULL!=buf) {
		sz = Pkt::DSZ;
	} else {
		last_error = EEPAPI_MALLOC;
	}

	return (unsigned char *)buf;
}//

unsigned char *
Pkt::getBuf(int size) {

	if (0==sz) {
		buf = (char *) malloc(size);
		if (NULL!=buf)
			sz=size;
		else {
			last_error = EEPAPI_MALLOC;
			return NULL;
		}
	}

	//requested size fits the
	//current size? else realloc
	unsigned char *tmp;
	if (size>sz) {
		tmp = (unsigned char *) realloc(buf, size);
		if (NULL!=tmp) {
			sz = size;
			buf=(char *)tmp;
		} else {
			last_error = EEPAPI_REALLOC;
		}
	}
	return (unsigned char *)buf;
}//

void
Pkt::setLength(int _len) {
	len = _len;
}

int
Pkt::getLength(void) {
	return len;
}

// =========================================
// PktHandler Class
// =========================================

PktHandler::PktHandler() {
	last_error=0;
	ifd = 0; //usually stdin
	ofd = 1; //usually stdout
}//

PktHandler::PktHandler(int _ifd, int _ofd) {
	last_error=0;
	ifd = _ifd;
	ofd = _ofd;
}

PktHandler::~PktHandler() {

}//


/**
 * @return >=0 len retrieved
 * @return <0  ERROR eg. EPIPE
 */
int
PktHandler::rx_exact(Pkt **p, int len) {

	unsigned char *buf;
	buf=(*p)->getBuf(len);

	int i, got=0;

	do {
		if ((i = read(ifd, buf+got, len-got)) < 0) {
			return i;
		}
		got += i;
	} while (got<len);

	return len;
}//

/**
 * @return 0 SUCCESS
 * @return 1 FAILURE
 */
int
PktHandler::rx(Pkt **p) {

	*p = new Pkt();

	//read length field first
	int result=PktHandler::rx_exact(p, 2);
	if (result<0) {
		last_error = EEPAPI_ERRNO; //check errno
		return 1;
	}

	unsigned char (*buf)[] = (unsigned char (*)[])(*p)->getBuf();
	int l = ((*buf)[0] << 8) | (*buf)[1];

	// the packet length gave us
	// the information we needed
	// to extract the right count
	// of bytes from the pipe
	result = PktHandler::rx_exact(p, l);
	if (result<0) {
		last_error = EEPAPI_ERRNO;
		return 1;
	}

	(*p)->setLength( result );

	return 0;
}//


int
PktHandler::tx(Pkt *p) {

	ei_x_buff *buf = p->getTxBuf();
	unsigned char li;

	int result;

	//write packet length
	//===================
	li = (buf->index >> 8) & 0xff;
	result = PktHandler::tx_exact((char *)&li, 1);
	if (result<=0) {
		last_error = EEPAPI_ERRNO;
		return 1;
	}

	li = buf->index & 0xff;
	result = PktHandler::tx_exact((char *)&li, 1);
	if (result<=0) {
		last_error = EEPAPI_ERRNO;
		return 1;
	}

	// write packet body
	result = PktHandler::tx_exact(buf->buff, buf->index);
	if (result<=0) {
		last_error = EEPAPI_ERRNO;
		return 1;
	}

	return 0;
}//

int
PktHandler::tx_exact(char *buf, int len) {

	int i, wrote = 0;

	do {
		if ((i = write(ofd, buf+wrote, len-wrote)) <= 0)
			return i;
		wrote += i;
	} while (wrote<len);

	return len;
}//
