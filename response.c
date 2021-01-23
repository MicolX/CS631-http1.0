/*
 * response.c
 * Builds a response based on the user's request and writes it back to them.
 *
 * Mingyao Xiong
 * Liam Brew
 *
 * Based on material from Stevens Institute of Technology's
 * CS 631 APUE during the Fall 2020 semester.
 */

#include "response.h"

#define MESSAGESIZE 256
#define OK 0
#define CREATED 1
#define ACCEPT 2
#define NO_CONTENT 3
#define MOVED_PERM 4
#define MOVED_TEMP 5
#define NOT_MODIFIED 6
#define BAD_REQUEST 7
#define UNAUTHORIZED 8
#define FORBIDDEN 9
#define NOT_FOUND 10
#define INTERNAL_ERR 11
#define NOT_IMP 12
#define BAD_GATEWAY 13
#define UNAVAILABLE 14

const char *status[] = {
	"200 OK\r\n",
	"201 Created\r\n",
	"202 Accepted\r\n",
	"204 No Content\r\n",
	"301 Moved Permanently\r\n",
	"302 Moved Temporarily\r\n",
	"304 Not Modified\r\n",
	"400 Bad Request\r\n",
	"401 Unauthorized\r\n",
	"403 Forbidden\r\n",
	"404 Not Found\r\n",
	"500 Internal Server Error\r\n",
	"501 Not Implemented\r\n",
	"502 Bad Gateway\r\n",
	"503 Service Unavailable\r\n"};

/*
 * Uses request data to build, if applicable, the appropriate response.
 */
int respond(char *rootpath, Request *req, Response *res)
{
	if (req->errcode != 0)
	{
		switch (req->errcode)
		{
		case 400:
			res->status = status[BAD_REQUEST];
			break;
		case 501:
			res->status = status[NOT_IMP];
			break;
		}
		res->headonly = 1;
		return 0;
	}

	struct stat sb;
	struct tm ifmtime;
	int fd;
	magic_t cookie;
	char uri[MAXPATHLEN];

	if (req->hastilde == 1)
	{
		(void)strncpy(uri, req->uri, MAXPATHLEN);
	}
	else
	{
		snprintf(uri, MAXPATHLEN, "%s/%s", rootpath, req->uri);
	}

	if (stat(uri, &sb) == -1)
	{
		switch (errno)
		{
		case EACCES:
			syslog(LOG_INFO, "Error: EACCES stat on URI");
			res->status = status[FORBIDDEN];
			break;
		default:
			syslog(LOG_INFO, "Error performing stat on URI");
			res->status = status[NOT_FOUND];
			break;
		}
		res->headonly = 1;
		return 0;
	}

	res->status = status[OK];

	if (S_ISDIR(sb.st_mode))
	{
		char htmlpath[MAXPATHLEN];

		snprintf(htmlpath, MAXPATHLEN, "%s/index.html", uri);

		if (access(htmlpath, F_OK) != 0)
		{
			res->dirindex = 1;
			strlcpy(req->uri, uri, MAXPATHLEN);
			return 0;
		}
		else
		{
			res->dirindex = 0;
			strlcpy(req->uri, htmlpath, MAXPATHLEN);
		}
	}
	else
	{
		res->dirindex = 0;
		strlcpy(req->uri, uri, MAXPATHLEN);
	}

	if (stat(req->uri, &sb) == -1)
	{
		syslog(LOG_INFO, "Error: stat on URI");
		return -1;
	}

	res->lastmtime = sb.st_mtime;

	if (strlen(req->ifms) > 0)
	{
		if (strptime(req->ifms, "%a, %d %b %Y %T GMT", &ifmtime) == NULL)
		{
			if (strptime(req->ifms, "%a, %d-%b-%Y %T GMT", &ifmtime) == NULL)
			{
				if (strptime(req->ifms, "%a %b  %d %T %Y", &ifmtime) == NULL)
				{
					res->status = status[BAD_REQUEST];
					res->headonly = 1;
					return 0;
				}
			}
		}

		if (difftime(sb.st_mtime, mktime(&ifmtime)) > 0)
		{
			res->headonly = 0;
		}
		else
		{
			res->status = status[NOT_MODIFIED];
			res->headonly = 1;
			return 0;
		}
	}

	if ((fd = open(req->uri, O_RDONLY)) < 0)
	{
		syslog(LOG_INFO, "Error opening URI");
		return -1;
	}

	res->contentlength = (long long)sb.st_size;

	if ((cookie = magic_open(MAGIC_MIME)) == NULL)
	{
		syslog(LOG_INFO, "magic_open error");
		(void)close(fd);
		return -1;
	}

	if (magic_load(cookie, NULL) != 0)
	{
		syslog(LOG_INFO, "magic_load error");
		(void)magic_close(cookie);
		(void)close(fd);
		return -1;
	}

	res->contenttype = magic_descriptor(cookie, fd);
	res->status = status[OK];
	if (req->method == HEAD)
	{
		res->headonly = 1;
	}
	return 0;
}

int(compar)(const FTSENT **a, const FTSENT **b)
{
	return strcmp((*a)->fts_name, (*b)->fts_name);
}

/*
 * Sends the response (including file transversal work, if applicable) back the client.
 */
int reply(int socket, Request *req, Response *res)
{
	char message[MESSAGESIZE];
	char buf[BUFSIZ];
	int fd;
	ssize_t readsize;
	ssize_t writesize;

	if (req->errcode != 0)
	{
		snprintf(message, sizeof(message), "HTTP/1.0 %s\r\n", res->status);
		if (write(socket, message, strlen(message)) != (signed int)strlen(message))
		{
			return -1;
		}
		return 0;
	}

	if (req->version == 1.0)
	{
		if (res->dirindex == 0 && strcmp(res->status, "200 OK\r\n") == 0)
		{
			char curtime[MAX_TIME];
			char mtime[MAX_TIME];
			struct tm *curtm;
			struct tm *mtm;
			time_t now;

			now = time(0);
			curtm = localtime(&now);
			strftime(curtime, sizeof(curtime), "%a, %d %b %Y %T GMT", curtm);

			mtm = localtime(&(res->lastmtime));
			strftime(mtime, sizeof(curtime), "%a, %d %b %Y %T GMT", mtm);

			snprintf(message, sizeof(message), "HTTP/1.0 %s\r\nDate: %s\r\nServer: SAS(Super Awesome Server)\r\nLast-Modified: %s\r\nContent-Type: %s\r\nContent-Length: %lld\r\n\r\n", res->status, curtime, mtime, res->contenttype, res->contentlength);
		}
		else
		{
			snprintf(message, sizeof(message), "HTTP/1.0 %s\r\n", res->status);
		}

		if (write(socket, message, strlen(message)) != (signed int)strlen(message))
		{
			syslog(LOG_INFO, "Error writing to socket");
			return -1;
		}
	}

	if (res->dirindex == 1 && req->method == GET)
	{
		FTSENT *ent;
		FTS *ftsp;
		char *dir[2];
		dir[0] = req->uri;
		dir[1] = NULL;

		ftsp = fts_open(dir, FTS_PHYSICAL, compar);
		errno = 0;
		while ((ent = fts_read(ftsp)))
		{
			if (errno)
			{
				syslog(LOG_INFO, "Error reading FTS");
				(void)fts_close(ftsp);
				return -1;
			}
			if (ent->fts_level == 1 && ent->fts_info != FTS_DP)
			{
				char fname[strlen(ent->fts_name) + 2];
				(void)snprintf(fname, sizeof(fname), "%s\n", ent->fts_name);

				if (write(socket, fname, strlen(fname)) != (int)strlen(fname))
				{
					syslog(LOG_INFO, "Error writing to socket on FTS");
					(void)fts_close(ftsp);
					return -1;
				}
			}
		}
		(void)fts_close(ftsp);
		return 0;
	}

	if (req->method == GET)
	{
		if ((fd = open(req->uri, O_RDONLY)) == -1)
		{
			syslog(LOG_INFO, "Error opening GET URI");
			return -1;
		}

		while ((readsize = read(fd, buf, sizeof(buf))) > 0)
		{
			if ((writesize = write(socket, buf, readsize)) != readsize)
			{
				syslog(LOG_INFO, "Error writing to socket on GET");
				(void)close(fd);
				return -1;
			}
		}

		(void)close(fd);
	}

	return 0;
}

/*
 * Executes any CGI requests.
 */
int runcgi(int socket, char *uri, char *dir)
{
	char *path;
	pid_t pid;

	path = strsep(&uri, "?");

	if ((pid = fork()) < 0)
	{
		syslog(LOG_INFO, "Error forking");
		return -1;
	}
	else if (pid == 0)
	{
		if (dup2(socket, STDOUT_FILENO) < 0)
		{
			syslog(LOG_ERR, "Error duping STDOUT_FILENO");
			exit(EXIT_FAILURE);
		}

		if (dup2(socket, STDERR_FILENO) < 0)
		{
			syslog(LOG_ERR, "Error duping STDERR_FILENO");
			exit(EXIT_FAILURE);
		}

		if (setenv("PATH", dir, 0) != 0)
		{
			syslog(LOG_ERR, "Failed to set PATH=%s", dir);
			err(EXIT_FAILURE, "Failed to set PATH=%s", dir);
		}

		if (uri != NULL && strlen(uri) > 0)
		{
			char *var;
			while ((var = strsep(&uri, "&")) != NULL)
			{
				if (putenv(var) == -1)
				{
					syslog(LOG_ERR, "Failed to set env %s", var);
					err(EXIT_FAILURE, "Failed to set env %s", var);
				}
			}
		}

		// execl("/bin/sh", "sh", "-c", command, (char*) 0);
		execl(path, "", (char *)0);

		return -1;
	}

	(void)wait(NULL);
	return 0;
}

/*
 * Handles directory pathing based on the presence of a user directory.
 */
int userdirhandler(char *uri, char newuri[])
{
	char *before, *uname;
	struct passwd *psw;

	before = strsep(&uri, "~");
	uname = strsep(&uri, "/");

	if ((psw = getpwnam(uname)) == NULL)
	{
		syslog(LOG_INFO, "Error on getpwnam()");
		return -1;
	}

	if (before != NULL && before[0] != '\0')
	{
		if (uri != NULL)
		{
			snprintf(newuri, MAXPATHLEN, "%s/%s/sws/%s", before, psw->pw_dir, uri);
		}
		else
		{
			snprintf(newuri, MAXPATHLEN, "%s/%s/sws", before, psw->pw_dir);
		}
	}
	else
	{
		if (uri != NULL)
		{
			snprintf(newuri, MAXPATHLEN, "%s/sws/%s", psw->pw_dir, uri);
		}
		else
		{
			snprintf(newuri, MAXPATHLEN, "%s/sws", psw->pw_dir);
		}
	}

	return 0;
}