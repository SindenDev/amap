# Amap Maps plugin for QtLocation
## 高德地图Qt插件 plugin for QtLocation module
* Qt地图插件，使用简单方便，不用浏览器内核，运行高效迅猛；
* 跨平台支持，（Qt Windows（MinGW, VS），Linux(桌面、嵌入式)， Android， ios, qnx都可以的而且测试过）；
* 在线跟新下载，支持离线地图；
* 由于我国家安全，GPS定位数据是加密过的，加入纠偏算法会有2m左右误差，精度要求高可以使用高德api进行在线纠偏；
* 经过测试的最新支持Qt5.13.2（Qt5.9相关接口调整已经做过适配，理论上Qt不更新相关API，可以支持到永远）；

## 编译工程
1.将在生成目录编译出插件（以使用MinGW为例子）
```
	plugins\geoservices\qtgeoservices_amap.dll 
	plugins\geoservices\qtgeoservices_amapd.dll
```

2.将（qtgeoservices_amap.dll 、qtgeoservices_amapd.dll）拷贝插件到Qt的 plugins\geoservices 定位插件目录
```
	C:\Qt\Qt5.13.2\5.13.2\mingw53_32\plugins\geoservices
```
## 配合使用QML建立地图
```
	import QtQuick 2.12
	import QtQuick.Window 2.12
	import QtPositioning 5.12
	import QtLocation 5.12

	Window {
		visible: true
		width: 640
		height: 480
		title: qsTr("Hello World")

		Map{
			anchors.fill:parent
			plugin: Plugin{name: "amap"}
		}
	}
```

### 想必这套代码会让你很受用，特别是使用Qt做嵌入式开发的你，目前很少有地图厂商做嵌入式的SDK，这个插件解决的痛点不光如此，还有在硬件资源稀缺的嵌入式平台，高性能，低延迟，支持触控和手势操作也唯有插件了，嵌入浏览器（非常吃内存，交互非常卡顿）是很得不偿失的方案，性能低一点的板子压根就做不了的，你老板知道的话肯定不会让你那么干，我把这么好的方案开源了， 不用谢！。
