# Crontab(1)

Linux에서 반복적인 작업을 자동으로 수행시킬 때 사용하는 명령어이다.

## Usage

```sh
$ crontab [-u user] file
$ crontab [-u user] [-i] {-e | -l | -r}
```

* `-e`: edit user's crontab
* `-l`: list user's crontab
* `-r`: delete user's crontab
* `-i`: prompt before deleting user's crontab

## Description

`$ crontab -e` 명령어를 입력한 후, 다음의 형식으로 원하는 작업을 명시한다.

단, 한 줄에 하나의 작업만 명시를 해야 한다.

Field 1 | Field 2 | Field 3 | Field 4 | Field 5 | Field 6
--------|---------|---------|---------|---------|--------
minute  | hour    | day     | month   | week    | command

* week
	+ 0 or 7 : Sunday
	+ 1 : Monday
	+ 2 : Tuesday
	+ 3 : Wednesday
	+ 4 : Thursday
	+ 5 : Friday
	+ 6 : Saturday

## Example

```sh
# 1분마다 start.sh을 실행한다.
> $ * * * * * /home/test/start.sh

# 매일 오전/오후 8:30분마다 start.sh을 실행한다.
> $ 30 8,20 * * * /home/test/start.sh

# 매일 매시간 10분, 20분마다 start.sh을 실행한다.
> $ 10,20 * * * * /home/test/start.sh

# 평일 오전 9시마다 start.sh을 실행한다.
> $ 0 9 * * 1-5 /home/test/start.sh

# 매일 오전 10:00부터 30분까지 1분 마다 start.sh를 실행한다.
> $ 0-30 10 * * * /home/test/start.sh

# 매일 30분마다 start.sh을 실행한다.
> $ */30 * * * * /home/test/start.sh
```
