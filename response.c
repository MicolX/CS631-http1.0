#include "response.h"


const char* status[] = {
	"200 OK\r\n",				// 0
	"201 Created\r\n",			// 1
	"202 Accepted\r\n",			// 2
	"204 No Content\r\n",		// 3
	"301 Moved Permanently\r\n",// 4
	"302 Moved Temporarily\r\n",// 5
	"304 Not Modified\r\n",		// 6
	"400 Bad Request\r\n",		// 7
	"401 Unauthorized\r\n",		// 8
	"403 Forbidden\r\n",		// 9
	"404 Not Found\r\n",		// 10
	"500 Internal Server Error\r\n",	// 11
	"501 Not Implemented\r\n",			// 12
	"502 Bad Gateway\r\n",				// 13
	"503 Service Unavailable\r\n"		// 14
};

int
respond(int rootfd, Request *req, Response *res) {
	if (req->errcode != 0) {
		switch (req->errcode) {
			case 400:
				res->status = status[7];
				break;
			case 501:
				res->status = status[12];
				break;
		}
		res->headonly = 1;
		return 0;
	}

	struct stat sb;
	struct tm ifmtime;
	int fd;
	magic_t cookie;
	
	if (fstatat(rootfd, req->uri, &sb, AT_SYMLINK_NOFOLLOW) == -1) {
		switch errno {
			case EACCES:
				res->status = status[9];
				break;
			default:
				res->status = status[10];
				break;
		}
		res->headonly = 1;
		return 0;
	}

	if (req->ifms != NULL) {
		if (strptime(req->ifms, "%a, %d %b %g %T GMT", &ifmtime) == NULL) {
			if (strptime(req->ifms, "%a, %d-%b-%g %T GMT", &ifmtime) == NULL) {
				res->status = status[7];
				res->headonly = 1;
				return 0;
			}
		}

		if (sb.st_mtime >= mktime(&ifmtime)) {
			res->lastmtime = sb.st_mtime;
			res->headonly = 0;
		} else {
			res->status = status[6];
			res->headonly = 1;
			return 0;
		}
	}

	res->contentlength = (long long)sb.st_size; 

	if ((fd = openat(rootfd, req->uri, O_RDONLY)) < 0) {
		// log error
		return -1;
	}

	if ((cookie = magic_open(MAGIC_MIME)) == NULL) {
		// log error
		(void)close(fd);
		return -1;
	}

	if (magic_load(cookie, NULL) != 0) {
		// log error
		(void)magic_close(cookie);
		(void)close(fd);
		return -1;
	}

	res->contenttype = magic_descriptor(cookie, fd);	
	res->status = status[0];
	return 0;
}
	
int
reply(int socket, int rootfd, Request *req, Response *res) {
	char message[256];
	char buf[BUFSIZ];
	int fd;
	ssize_t readsize;
	ssize_t writesize;
	
	if (res->headonly) {
		snprintf(message, sizeof(message), "HTTP/1.0 %s\r\n", res->status);
		if (write(socket, message, strlen(message)) != (signed int)strlen(message)) {
			// log error
			return -1;
		}
		return 0;
	}

	if (req->version == 1.0) {
		char curtime[32];
		char mtime[32];
		struct tm *curtm;
		struct tm *mtm;
		time_t now;
		
		now = time(0);
		curtm = localtime(&now);
		strftime(curtime, sizeof(curtime), "%a, %d %b %g %T GMT", curtm);

		mtm = localtime(&(res->lastmtime));
		strftime(mtime, sizeof(curtime), "%a, %d %b %g %T GMT", mtm);

		snprintf(message, sizeof(message), "HTTP/1.0 %s\r\nDate: %s\r\nServer: SAS(Super Awesome Server)\r\nLast-Modified: %s\r\nContent-Type: %s\r\nContent-Length: %lld\r\n\r\n", res->status, curtime, mtime, res->contenttype, res->contentlength);

		if (write(socket, message, strlen(message)) != (signed int)strlen(message)) {
			// log error
			return -1;
		}
	}

	if ((fd = openat(rootfd, req->uri, O_RDONLY)) == -1) {
		// log error
		return -1;
	}

	while ((readsize = read(fd, buf, sizeof(buf))) > 0) {
		if ((writesize = write(socket, buf, sizeof(buf))) != readsize) {
			// log error
			(void)close(fd);
			return -1;
		}
	}

	(void)close(fd);
	return 0;
}
