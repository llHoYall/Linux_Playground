# Permission

Linux에서 권한 설정은 다음 3개의 group에 가능하다.

* File을 소유하는 사용자
* File을 소유하는 group에 속한 사용자
* 그 외의 사용자

권한의 종류는 다음 3가지다.

* Read (r)
* Write (w)
* Execute (x)

`ls -l` 등의 명령어를 사용하면 권한 확인이 가능하다. 표시 형식은 `drwxrwxrwx`와 같은 형식인데, 다음의 의미를 갖는다.

```
소유 사용자 | 소유 group | 그 외
```

주로 사용되는 예는 다음과 같다.

* rw-r--r-- : 소유자만 읽고 쓸 수 있고, 그 외의 사용자는 읽기만 가능하다. 일반적으로 사용된다.
* rwxr-xr-x : 소유자는 읽고 쓰고 실행할 수 있고, 그 외의 사용자는 읽고 실행만 할 수 있다. 프로그램이나 directory에 사용된다.
* rw------- : 소유자만 읽고 쓸 수 있다. SSH의 비밀 키 등의 file에 사용된다.

## Directory Permission

Directory의 권한은 다음과 같이 적용된다. Directory를 그 안에 담긴 file list를 기록한 file이라고 생각하면 이해하기 쉽다.

* r : `ls` 명령어 등으로 directory의 file 목록을 확인할 수 있다.
* w : 새로운 file을 안에 쓰거나 안에 있는 file을 삭제할 수 있다.
* x : 안에 있는 file에 접근할 수 있다.
