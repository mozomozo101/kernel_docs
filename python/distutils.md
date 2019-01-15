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
buildコマンドによって生成されたファイルは、デフォルトでは、実行したディレクトリ配下に置かれる。
```
--- build/ --- lib/
or
--- build/ --- lib.<plat>/    // 拡張モジュール
               temp.<plat>/   // 中間生成オブジェクトなど
```

なお、build時に、配置するディレクトリを指定することもできる。
```
python setup.py build --build-base=/path/to/pybuild/foo-1.0
```

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
