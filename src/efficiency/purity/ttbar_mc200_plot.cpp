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

    // PDF 出力ファイル名
    string pdfFileName = string(basePath) + "/output/" + string(dir) + "/ttbar200_comparison.pdf";

    // キャンバスの作成（1ページ目：efficiency vs purity）
    TCanvas *c1 = new TCanvas("c1", "", 800, 600);
    
    // 凡例の作成
    TLegend *legend = new TLegend(0.15, 0.15, 0.45, 0.35);
    legend->SetBorderSize(0); // 枠線をなくす
    legend->SetTextSize(0.03);

    vector<TGraph*> graphs;

    // 各ファイルを TGraph に読み込み、設定を行う
    for (size_t i = 0; i < fileNames.size(); ++i) {
        string filePath = txtDir + fileNames[i];
        TGraph *graph = new TGraph(filePath.c_str(), "%lg %lg");
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

    // 1ページ目の PDF を出力
    c1->Print((pdfFileName + "(").c_str());

    // 2ページ目のキャンバスを作成（bin_width vs failed_entries のグラフ）
    TCanvas *c2 = new TCanvas("c2", "", 900, 600);

    // 2ページ目のグラフ用の凡例
    TLegend *legend2 = new TLegend(0.15, 0.15, 0.45, 0.35);
    legend2->SetBorderSize(0); // 枠線をなくす
    legend2->SetTextSize(0.03);

    vector<TGraph*> graphs_failed;

    // 各ファイルを TGraph として読み込み、bin_width vs failed_entries のグラフを作成
    for (size_t i = 0; i < fileNames.size(); ++i) {
        string filePath = txtDir + fileNames[i];
        
        TGraph *graph_failed = new TGraph(filePath.c_str(), "%lg %*lg %*lg %lg");
        graph_failed->SetMarkerColor(colors[i]);
        graph_failed->SetLineColor(colors[i]);
        graph_failed->SetLineWidth(2);
        graph_failed->SetMarkerStyle(21);
        graph_failed->SetTitle("");

        if (i == 0) {
            graph_failed->GetXaxis()->SetTitle("efficiency");
            graph_failed->GetYaxis()->SetTitle("failed_entries");
            graph_failed->GetXaxis()->SetTitleSize(0.04);
            graph_failed->GetYaxis()->SetTitleSize(0.04);
            graph_failed->GetXaxis()->SetLabelSize(0.04);
            graph_failed->GetYaxis()->SetLabelSize(0.04);
            graph_failed->Draw("ALP");  // 最初のグラフを描画
        } else {
            graph_failed->Draw("LP SAME");  // 同じキャンバスに重ねて描画
        }

        legend2->AddEntry(graph_failed, labels[i].c_str(), "lp");  // 凡例に追加
        graphs_failed.push_back(graph_failed);
    }

    legend2->Draw();

    c2->Print((pdfFileName + ")").c_str());

    // メモリ解放
    delete c1;
    delete c2;
    for (TGraph* graph : graphs) {
        delete graph;
    }
    for (TGraph* graph_failed : graphs_failed) {
        delete graph_failed;
    }
    delete legend;
    delete legend2;
}
