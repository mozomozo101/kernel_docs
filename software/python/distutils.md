https://docs.python.jp/3/install/index.html

# Distutils
pythonには、モジュールを配布する際の標準化された仕組みとしてdisutilsがある。
モジュールのディレクトリ構成、インストール方法などを標準化したもののようだ。
distutilsに従ったモジュールでは、インストールは、
```
python setup.py install
```
で行う。

setup.pyは、installとbuildを別々に行うこともできる。
```
python setup.py build
python setup.py install
```

## build

### モジュールのビルドとは
https://mail.python.org/pipermail/python-list/2006-March/376618.html

python setup.py build は、.pyファイルから.pycファイルを生成する。

pythonでは、実行時に、.pyファイルをバイトコードである.pycにコンパイルする。
.pycには、使用するpythonのバージョンに応じた、モジュールのインポートに関する処理や情報が含まれているようだ。
このファイルは、pythonモジュールの初回実行時に生成され、以降はこれが使われるようになる。
そのため、事前にこれを生成しておくと、**初回実行時のみ** モジュールのインポートが高速化される。
以上より、.pycファイルは絶対必要なわけではない。

「コンパイル」というと、.pycファイルがアーキテクチャ依存であるかのように聞こえるが、そうではない。
単に、pythonのバージョンに依存するだけ。


### 動き
buildコマンドにより、.pyや.pycファイルが、実行したディレクトリ配下に置かれる。
[CやC++で書かれた拡張モジュール](https://docs.python.jp/3/extending/extending.html)が含まれる場合は、`[lib,temp].<plat>`に入る。
```
--- build/ --- lib/
or
--- build/ --- lib.<plat>/    // 拡張モジュール（.py, .pyc）
               temp.<plat>/   // 中間生成オブジェクトなど
```

なお、build時に、配置するディレクトリを指定することもできる。
```
python setup.py build --build-base=/path/to/pybuild/foo-1.0
```
※ CやC++で書かれた拡張モジュール: C等で書いたモジュールを組み込むことで、pythonが直接実行できないような処理を行える。

## install
`build/lib/`や`build/lib.<plat>`配下のファイルを、標準的なサードパーティ製モジュールの配置場所にコピーする。
python自体のインストールの仕方によって変わってくるが、unixの場合、以下になるはず。
```
/usr/local/lib/pythonX.Y/site-packages
```

インストール位置の指定には、いくつか方法がある。

### homeスキーム
```
python setup.py install --home=<dir>
```
指定したディレクトリに配置。

### userスキーム
```
python setup.py install --user
```
コマンドを実行するユーザのbaseディレクトリに配置される。
baseディレクトリはpythonによって定義されており、`site.USER_BASE`（デフォルトでは、`~/.local`）に置かれる。

### prefixスキーム
homeスキームに似てるけど、なんか違うらしい。今は調べない。
