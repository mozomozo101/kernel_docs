# TEE

https://globalplatform.org/wp-content/uploads/2018/05/Introduction-to-Trusted-Execution-Environment-15May2018.pdf

TEEとは、重要なデータの処理や保存などを行なうためのプロセッサ上の領域のこと。
ここはLinuxなどのリッチOSから隔離されているため、そこで攻撃が行われても、影響を受けない。
今までTEEと呼ばれるものが、いろんなところで作られてきたが、標準化されてないため不都合が多かった。

GPは、TEEの標準化のために必要な事柄として、以下を策定した。
* リッチOSからの隔離
* TAの隔離（他TA、TEEからも干渉されない）
* TAとTEEの修正は、認証された存在からしかなされない
* セキュアなブートプロセス
* TAやTEEのデータを安全に保存できる Trusted Storage
* TEEの制御下にあるペリフェラルへのアクセスをセキュアにするためのAPI
* 各種暗号エンジン

## TEEに関するドキュメントについて
* TEE System Architecture 
	* TEEの仕様について
* TEE APIs
	* APIの説明
* TEE Management Framework
	* TEE環境のライフサイクルについて。
	* TEE関連の仕様の中で核となるもの
	* プロビジョニング、TAの管理なども含める
* TEE Initial Configuration
	* TEEが必要とすデバイスの仕様など？
* TEE Protection Profile
  * TEE環境やそのデバイスが耐えられるべき脅威について
 
