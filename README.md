# Amap Maps plugin for QtLocation
## 高德地图插件 plugin for QtLocation module
* Qt地图插件
* 跨平台支持
* 最新支持Qt5.10.0
## 编译本工程
1.将生成插件

plugins\geoservices\qtgeoservices_amap.dll 
plugins\geoservices\qtgeoservices_amapd.dll

2.目录拷贝插件到Qt安装目录

（如C:\Qt\Qt5.10.0\5.10.0\mingw53_32\plugins\geoservices）
## 建立地图
	import QtPositioning 5.8;
	import QtLocation 5.9;
	Map{
    	plugin: Plugin{ name: "amap"}
	}
```
