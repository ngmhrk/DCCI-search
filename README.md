# DCCI-search

Directional Cubic Convolution Interpolation (DCCI) の性能向上を目的として，補間処理における参照画素の選び方を探索したレポジトリです．
一部研究室のコードを利用しており，本公開レポジトリから除外しているため，ビルドは通りません．

## 実行方法

### 環境

- Halide: 13.0.4
- OpenCV: 4.5.0
- OpenCP: https://github.com/norishigefukushima/OpenCP

### 準備

以下のライブラリを利用しています
- OpenCV
  - 概要: 画像処理ライブラリ
  - URL: https://opencv.org/
- Halide
  - 概要: 画像処理に特化したドメイン固有言語
  - URL: https://halide-lang.org/
- OpenCP
  - 概要: 画像処理ライブラリ（研究室内向け）
  - URL: https://github.com/norishigefukushima/OpenCP


以下の環境変数を使用しています。
- OPENCV_INCLUDE_DIR: OpenCVのincludeディレクトリへのPath
- OPENCV_LIB_DIR: OpenCVのlibディレクトリへのPath
- HALIDE_INCLUDE_DIR: HalideのincludeディレクトリへのPath
- HALIDE_LIB_DIR: HalideのlibディレクトリへのPath
- HALIDE_BIN_DIR: HalideのbinディレクトリへのPath
- OPENCP_INCLUDE_DIR: OpenCPのincludeディレクトリへのPath
- OPENCP_LIB_DIR: OpenCPのlibディレクトリへのPath

### 手順

#### Halideで記述した画像処理プログラムのビルド

1. generatorプロジェクトをビルド
    - generator/tools/GenGen.cpp内のmain関数が実行され、staticライブラリを生成するための実行ファイル(x64/Release/generator.exe)が作成される
2. rootディレクトリにあるgenerate.ps1を実行
    - 以下の2つの処理を行うスクリプト
        - 出力ディレクトリ(generator/dest/host)の作成
        - generatorプロジェクトのビルドによって生成された実行ファイルを引数を変えながら実行
    - オートスケジューラによるスケジューリング(3パターン)の関数が生成される

#### 実験

- testPSNR: PSNRの計測実験
- testSSIM: SSIMの計測実験
- createImage: アップサンプル後の画像を保存し，保存した画像を表示
- benchmark: 実行時間の計測実験
- searchDCCILocalOptimizePSNR: 初期値を与えてPSNRを評価指数として局所最適化を行う
- searchDCCILocalOptimizeSSIM: 初期値を与えてSSIMを評価指数として局所最適化を行う
