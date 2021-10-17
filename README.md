# nxtOSEK 초음판 센서를 활용한 Follower
아주대학교 2021년 가을학기 OSEK/VDX 프로그래밍 팀 프로젝트

## OSEK/VDX 을 이용한 follow 차량 개발
-nxtOSEK: [nxtOSEK Installation in Linux](http://lejos-osek.sourceforge.net/installation_linux.htm)
</br>
![BareMetal vs OSEK/VDX](https://github.com/jangByeongHui/nxtOSEK/blob/main/asset/OSEK.jpg?raw=true)

### 적용 알고리즘
- Low Pass Filter : [Low Pass Filter](https://ko.wikipedia.org/wiki/%EB%A1%9C%EC%9A%B0%ED%8C%A8%EC%8A%A4_%ED%95%84%ED%84%B0)
- Proportional-Integral-Differential controller(PID) : [PID](https://ko.wikipedia.org/wiki/PID_%EC%A0%9C%EC%96%B4%EA%B8%B0)
* Low Pass Filter는 초음파 센서의 간섭 및 값이 튀는 현상을 보정하기 위해서 적용
* PID는 앞 차량과의 거리에 따라 차량의 속력을 조절하고 조향 같은 경우에도 판의 위치에 따라 조향을 하기 위해 적용

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

### 결론
- 차량이 회전을 한후 조향을 정위치 시키는 것이 중요하다.
- 따라가는 판의 모형이 볼록한 경우 차량이 중심을 향해 이동하기에 고속에서 잘 따라온다.