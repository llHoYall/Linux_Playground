# User

Linux kernel은 사용자 ID를 기반으로 동작한다. 사용자 이름과 사용자 ID는 서로 변환되어 사용된다.

사용자 정보는 일반적으로 `/etc/passwd`에 저장되어진다. 예를 들면 다음과 같다.

```
hoya128:x:1001:1002::/home/hoya128:/usr/bin/zsh
```

* 첫 번째 항목은 사용자 이름이다.
* 두 번째 항목이 `x`이면 비밀번호가 `/etc/shadow`에 저장되어있음을 의미한다.
* 세 번째 항목은 사용자 ID이다.
* 네 번째 항목은 사용자가 속한 group의 ID이다. Group 정보는 `/etc/group`에 저장되어 있다.

NIS(Network Information Service)나 LDAP(Lightweight Directory Access Protocol)과 같은 system을 사용하여 복수의 machine에서 사용자 정보를 공유하는 경우가 있으므로, 사용자 정보를 다룰 때는 `/etc/passwd`나 `/etc/group`을 직접 접근하지 말고 전용 API를 사용해야 한다.
