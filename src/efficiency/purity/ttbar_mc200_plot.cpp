#include <iostream>
#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TLatex.h>
#include <TLegend.h>

using namespace std;

const char* basePath = getenv("WORKDIR");
const char* dir = "efficiency/purity";
const char* dataDir = "src/efficiency/data";

void ttbar_mc200_plot() {
    string txtDir = string(basePath) + "/" + string(dataDir) + "/";
    
    // 各しきい値のファイル名
    vector<string> fileNames = {
        "ttbar_mc200_0.000000.txt",
        "ttbar_mc200_0.200000.txt",
        "ttbar_mc200_0.100000.txt",
        "ttbar_mc200_0.050000.txt"
    };
    
    // しきい値ごとのラベル名
    vector<string> labels = {
        "No threshold",
        "pT threshold = 0.2",
        "pT threshold = 0.1",
        "pT threshold = 0.05"
    };
    
    // グラフの色設定
    vector<int> colors = {kBlue, kRed, kGreen, kMagenta};

    TCanvas *c1 = new TCanvas("c1", "", 800, 600);
    
    // 凡例の作成
    TLegend *legend = new TLegend(0.15, 0.15, 0.45, 0.35);
    legend->SetBorderSize(0); // 枠線をなくす
    legend->SetTextSize(0.03);

    vector<TGraph*> graphs;

    // 各ファイルを TGraph に読み込み、設定を行う
    for (size_t i = 0; i < fileNames.size(); ++i) {
        string filePath = txtDir + fileNames[i];
        TGraph *graph = new TGraph(filePath.c_str());
        graph->SetMarkerColor(colors[i]);
        graph->SetLineColor(colors[i]);
        graph->SetLineWidth(2);
        graph->SetMarkerStyle(21);
        graph->SetTitle("");
        
        if (i == 0) {
            graph->GetXaxis()->SetTitle("efficiency");
            graph->GetYaxis()->SetTitle("purity");
            graph->GetXaxis()->SetTitleSize(0.04);
            graph->GetYaxis()->SetTitleSize(0.04);
            graph->GetXaxis()->SetLabelSize(0.04);
            graph->GetYaxis()->SetLabelSize(0.04);
            graph->Draw("ALP");  // 最初のグラフを描画
        } else {
            graph->Draw("LP SAME");  // 同じキャンバスに重ねて描画
        }
        
        legend->AddEntry(graph, labels[i].c_str(), "lp");  // 凡例に追加
        graphs.push_back(graph);
    }

    // 凡例の表示
    legend->Draw();

    // ATLASラベルを追加
    TLatex latex;
    latex.SetNDC();
    latex.SetTextFont(72); // ATLASフォントスタイル
    latex.SetTextSize(0.04);
    latex.DrawLatex(0.2, 0.5, "ATLAS");

    latex.SetTextFont(42); // 標準のフォントスタイルに戻す
    latex.DrawLatex(0.32, 0.5, "Simulation Work in Progress");
    latex.DrawLatex(0.2, 0.45, "#sqrt{s} = 14 TeV");

    // 出力 PDF ファイル名
    c1->Print((string(basePath) + "/output/" + string(dir) + "/ttbar200_comparison.pdf").c_str());

    // メモリ解放
    delete c1;
    for (TGraph* graph : graphs) {
        delete graph;
    }
    delete legend;
}
