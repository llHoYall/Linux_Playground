# Socket API

`socket(2)`은 socket을 만드는 system call이다. `socket()`은 socket을 만들고 이에 대응하는 file descriptor를 반환한다.

```c
#include <sys/socket.h>
#include <sys/types.h>

int socket(int domain, int type, int protocol);
```

`connect(2)`는 socket에서 stream을 꺼내서 지정한 주소의 server에 stream을 연결한다. `connect()`에서는 반드시 host 이름이 아니라 IP 주소와 port 번호로 지정해야 한다.

```c
#include <sys/socket.h>
#include <sys/types.h>

int connect(int sock, const struct sockaddr* addr, socklen_t addrlen);
```

`bind(2)`는 접속을 기다리는 주소를 socket에 할당하는 system call이다.

```c
#include <sys/socket.h>
#include <sys/types.h>

int bind(int sock, struct sockaddr* addr, socklen_t addrlen);
```

`listen(2)`은 socket이 server용 socket, 즉 접속을 기다리는 socket임을 kernel에 알린다.

```c
#include <sys/socket.h>

int listen(int sock, int backlog);
```

`accept(2)`는 socket에 client가 접속하는 것을 기다리다 접속이 완료되면 연결된 stream의 file descriptor를 반환한다.

