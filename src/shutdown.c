/*!\file shutdown.c
 * BSD shutdown().
 */

/*  BSD sockets functionality for Watt-32 TCP/IP
 *
 *  Copyright (c) 1997-2002 Gisle Vanem <gvanem@yahoo.no>
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. All advertising materials mentioning features or use of this software
 *     must display the following acknowledgement:
 *       This product includes software developed by Gisle Vanem
 *       Bergen, Norway.
 *
 *  THIS SOFTWARE IS PROVIDED BY ME (Gisle Vanem) AND CONTRIBUTORS ``AS IS''
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED.  IN NO EVENT SHALL I OR CONTRIBUTORS BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  Version
 *
 *  0.5 : Dec 18, 1997 : G. Vanem - created
 */

#include "socket.h"

#if defined(USE_BSD_API)

static void shutdown_read (Socket *socket);
static void shutdown_write (Socket *socket);

int W32_CALL shutdown (int s, int how)
{
  Socket *socket = _socklist_find (s);

#if defined(USE_DEBUG)
  static char fmt[] = "\nshutdown:%d/??";
  static char rrw[] = "r w rw";

  how &= 3;
  fmt [sizeof(fmt)-3] = rrw [2*how];
  fmt [sizeof(fmt)-2] = rrw [2*how+1];
#endif

  SOCK_PROLOGUE (socket, fmt, s);

  switch (how)
  {
    case SHUT_RD:
         shutdown_read (socket);
         return (0);

    case SHUT_WR:
         shutdown_write (socket);
         return (0);

    case SHUT_RDWR:
         shutdown_read (socket);
         shutdown_write (socket);
         return (0);
  }

  SOCK_ERRNO (EINVAL);
  return (-1);
}

static void shutdown_read (Socket *socket)
{
  socket->so_state   |=  SS_CANTRCVMORE;
  /* socket->so_options &= ~SO_ACCEPTCONN; */

  switch (socket->so_type)
  {
    case SOCK_PACKET:
    case SOCK_RAW:
    case SOCK_DGRAM:
         break;

    case SOCK_STREAM:
         /** \todo For tcp, should send RST if we get
          *        incoming data. Don't send ICMP error.
          */
  }
}

static void shutdown_write (Socket *socket)
{
  socket->so_state   |=  SS_CANTSENDMORE;
  socket->so_state   &= ~SS_ISLISTENING;
  /* socket->so_options &= ~SO_ACCEPTCONN; */

  switch (socket->so_type)
  {
    case SOCK_PACKET:
    case SOCK_RAW:
    case SOCK_DGRAM:
         break;

    case SOCK_STREAM:
         /* Send FIN as soon as possible. */
         _tcp_close (socket->tcp_sock);
  }
}

#endif  /* USE_BSD_API */
