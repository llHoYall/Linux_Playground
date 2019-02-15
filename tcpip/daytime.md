# Daytime Protocol

The Daytime Protocol is a service in the Internet Protocol Suite, defined in 1983 in RFC 867. It is intended for testing and measurement purposes in computer networks.

A host may connect to a server that supports the Daytime protocol on either TCP or UDP port 13. The server returns an ASCII character string of the current date and time in an unspecified format.

On UNIX-like operating systems a daytime server is usually built into the inetd (or xinetd) daemom. The service is usually not enabled by default. It may be enabled by adding the following lines to the file `/etc/inetd.conf` and telling inetd to reload its configuration.

```sh
daytime  stream  tcp  nowait  root  internal
daytime  dgram   udp  wait    root  internal
```

An example output may be

> Monday, January 20, 2019 22:35:59-PST

## Configuration

`daytime` protocol을 사용하려면 `daytime` server를 구동해야 한다. 이것은 `inetd`와 `xinetd`에 포함된 program이다. `xinetd`는`inetd`를 보안과 관련된 부분을 개선한 개량 version이다.

다음과 같이 `daytime` server를 구동한다.

### Ubuntu

`xinetd`를 설치한다. Ubuntu의 경우 `xinetd`를 설치하면 바로 구동된다.

> $ sudo apt install xinetd

`daytime` protocol을 설정한다.

`/etc/xinetd.d/daytime` 파일을 관리자 권한으로 열어 TCP version의 daytime option을 다음과 같이 바꿔준다. 

> `disable=no`

`xinetd`를 reload 한다.

> $ sudo systemctl reload xinetd

### CentOS

Ubuntu를 참고하여 다음의 명령어로 수행한다.

`xinetd`를 설치한다.

> $ sudo yum install xinetd

설정 파일의 위치는 다음과 같다.

`/etc/xinetd.d/daytime-stream`

CentOS의 경우 `xinetd`를 설치해도 바로 구동되지 않으므로 직접 구동시킨다.

> sudo systemctl start xinetd

## Run

> $ mkdir build && cd build && cmake .. && make

## Usage

* `$ ./daytime`

