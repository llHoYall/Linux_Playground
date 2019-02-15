# Host Name

Host 이름은 `/etc/hosts`에 기록한다.

Host name과 IP 주소를 교환해주는 것을 `resolver`라고 한다. Linux에서는 IP 주소의 resolver로 libc가 있고, 해당 설정은 `/etc/nsswitch.conf`에 기술된다.

