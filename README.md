# PHYTHON学习笔记

pyinstaller 打包文件,打包后配置文件的工作目录可能会改变,需要在代码中动态地确定资源文件的路径,可以参考添加以下代码:  

```python
import sys
import os
# 判断程序是不是用 PyInstaller 打包的
if getattr(sys, 'frozen', False):
    # 如果是，使用这个路径
    application_path = sys._MEIPASS
else:
    # 如果不是，使用普通的Python环境路径
    application_path = os.path.dirname(os.path.abspath(__file__))

data_path = os.path.join(application_path, 'data.json')

# 现在可以使用 data_path 来访问 data.json 文件
with open(data_path, 'r') as file:
    data = file.read()
```

这段代码首先检查程序是否被PyInstaller打包（通过检查 sys.frozen）。如果是，它使用 sys._MEIPASS 目录作为应用程序的工作路径。这个目录是PyInstaller创建的临时目录，用于存储所有包含的资源文件。如果程序没有被打包，它就使用脚本的当前目录。

---