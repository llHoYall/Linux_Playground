# Credential

Linux 상에서 활동하는 주체는 사람이 아니라 process이다. Process가 가지는 사용자의 속성을 credential(자격 증명)이라고 한다.

자격 증명이란, 이 process는 linux상에서 이 사용자의 대리인으로 동작하고 있다는 증명서다. 다시 말하면, 증명서만 있으면 누가 생성한 process라도 kernel은 그 사용자의 대리인으로 인정한다는 말이다.

Linux에 login을 하면, 사용자의 증명서를 가진 process가 system 상에 생성된다. 이 최초의 process가 다른 process를 실행할 때 증명서를 자동으로 복사하고 전달한다. 따라서 login해서 생성한 process는 모두 해당 사용자의 대리인 자격을 가지고 실행된다.
