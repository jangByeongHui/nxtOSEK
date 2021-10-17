# nxtOSEK 초음판 센서를 활용한 Follower
아주대학교 2021년 가을학기 OSEK/VDX 프로그래밍 팀 프로젝트

## OSEK/VDX 방식으로 레고 마인드 스톰 프로그래밍
-nxtOSEK: [nxtOSEK Installation in Linux](http://lejos-osek.sourceforge.net/installation_linux.htm)
</br>
![BareMetal vs OSEK/VDX](https://github.com/jangByeongHui/nxtOSEK/blob/main/asset/OSEK.jpg?raw=true)


### 실행 방법
Ubuntu
``` bash
cd follower_origin #cd follower_test

make all

cp -r follower_jbh.rxe /ShareFolder/follower_jbh.rxe #cp -r follower_test.rxe /ShareFolder/follower_test.rxe
```

CMD
``` bash
NextTool.exe /COM=usb –versions #본체와 연결 확인
NextTool.exe /COM=usb -firmware=lms_arm_nbcnxc_128.rfw #펌웨어 업데이트
NextTool.exe /COM=usb –download=[자신이 만든 이미지 이름].rxe #어플리케이션 다운로드 예) NextTool.exe /COM=usb –download=helloworld_OSEK.rxe
NeXTTool.exe /COM=usb -listfiles=[자신이 만든 이미지 이름].rxe #다운로드 확인 예) NeXTTool.exe /COM=usb -listfiles=ResourceTest.rx
```



### 시연 영상
![초기 시연](https://github.com/jangByeongHui/nxtOSEK/blob/main/asset/demo.gif)

<img src="https://github.com/jangByeongHui/nxtOSEK/blob/main/asset/adjust_panel.gif" width="450" height="600">