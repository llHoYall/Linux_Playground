# Manual

Linux상에서 특정 명령어나 함수에 관해 검색할 때는 `man` 명령어를 사용한다.

> $ man \<command\>

## Section

`man` page는 내용에 따라 여러 section으로 구성되어 있다.

`man <command>`와 같이 사용하면 section 1부터 차례로 검색해서 가장 먼저 발견한 section의 page를 표시해주고, `man [section] <command>`와 같이 사용하면 해당 section에서만 검색한다.

1. 실행 가능한 program이나 shell 명령어
2. System call
3. Library 함수
4. 특별한 file들(Device file 등)
5. File format
6. Game
7. 규격 등
8. System 관리용 명령어

