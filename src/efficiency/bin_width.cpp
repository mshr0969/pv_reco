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
const char* rootFilePath = getenv("REAL_DATA_DIR");
const char* mcRootFilePath = getenv("MC_DATA_DIR");
const char* dir = "efficiency";

tuple<vector<vector<double>>, vector<double>, double> pv_reco(TTree *tree, bool is_ftf, int bin_num) {
    vector<double> *id_trk_pt = nullptr;
    vector<float> *id_trk_z0 = nullptr;
    vector<int> *vxp_type = nullptr;
    vector<float> *vxp_z = nullptr;
    vector<vector<double>> reco_z0;
    vector<double> primary_vertexies;
    double bin_width;

    const char* ptBranch = is_ftf ? "ftf_id_trk_pt" : "id_trk_pt";
    const char* z0Branch = is_ftf ? "ftf_id_trk_z0" : "id_trk_z0";

    tree->SetBranchAddress(ptBranch, &id_trk_pt);
    tree->SetBranchAddress(z0Branch ,&id_trk_z0);
    tree->SetBranchAddress("vxp_type", &vxp_type);
    tree->SetBranchAddress("vxp_z", &vxp_z);

    int entries = tree->GetEntries();

    float min_z0 = numeric_limits<float>::max();
    float max_z0 = numeric_limits<float>::lowest();

    for (int entry = 0; entry < entries; entry++) {
        tree->GetEntry(entry);

        for (size_t i = 0; i < id_trk_z0->size(); ++i) {
            min_z0 = min(min_z0, id_trk_z0->at(i));
            max_z0 = max(max_z0, id_trk_z0->at(i));
        }
    }

    float range = max_z0 - min_z0;
    float margin = 0.1 * range; // 10%のマージンを追加
    min_z0 -= margin;
    max_z0 += margin;

    for (int entry = 0; entry < entries; entry++) {
        tree->GetEntry(entry);

        TH1D *tempHist = new TH1D("tempHist", "Temporary Histogram", bin_num, min_z0, max_z0);

        for (size_t i = 0; i < id_trk_pt->size(); ++i) {
            tempHist->Fill(id_trk_z0->at(i), id_trk_pt->at(i));
        }

        int maxBin = tempHist->GetMaximumBin();
        double bin_low_edge = tempHist->GetXaxis()->GetBinLowEdge(maxBin);
        double bin_up_edge = tempHist->GetXaxis()->GetBinUpEdge(maxBin);
        reco_z0.push_back({bin_low_edge, bin_up_edge});
        bin_width = bin_up_edge - bin_low_edge;

        delete tempHist;

        float primary_vertex_z = 0;
        for (size_t i = 0; i < vxp_z->size(); ++i) {
            if (vxp_type->at(i) == 1) {
                primary_vertex_z = vxp_z->at(i);
                primary_vertexies.push_back(primary_vertex_z);
                break;
            }
        }
    }

    return make_tuple(reco_z0, primary_vertexies, bin_width);
}

void bin_width() {
    string fullPath = string(basePath) + "/" + string(mcRootFilePath);

    vector<vector<double>> reco_z0;
    vector<double> primary_vertexies;
    double bin_width;

    TGraph *g1 = new TGraph();
    g1->GetXaxis()->SetTitle("bin width [mm]");
    g1->GetYaxis()->SetTitle("efficiency");
    g1->GetXaxis()->SetTitleSize(0.04);
    g1->GetYaxis()->SetTitleSize(0.04);
    g1->GetXaxis()->SetLabelSize(0.04);
    g1->GetYaxis()->SetLabelSize(0.04);
    g1->SetMarkerColor(kBlue);
    g1->SetLineColor(kBlue);
    g1->SetLineWidth(4);

    TGraph *g2 = new TGraph();
    g2->SetMarkerColor(kOrange);
    g2->SetLineColor(kOrange);
    g2->SetLineWidth(4);

    int max_bin_num = 65536;
    int min_bin_num = 256;

    // FTF=true
    TFile *file = new TFile(fullPath.c_str());
    TTree *tree = dynamic_cast<TTree*>(file->Get("physics"));
    for (int bin_num = min_bin_num; bin_num <= max_bin_num; bin_num *= 2) {
        tie(reco_z0, primary_vertexies, bin_width) = pv_reco(tree, true, bin_num);

        int true_count = 0;
        int false_count = 0;

        for (size_t i = 0; i < reco_z0.size(); ++i) {
            if (reco_z0[i][0] < primary_vertexies[i] && primary_vertexies[i] < reco_z0[i][1]) {
                true_count++;
            } else {
                false_count++;
            }
        }

        double efficiency = static_cast<double>(true_count) / (true_count + false_count);
        g1->SetPoint(g1->GetN(), bin_width, efficiency);

        cout << "[FTF] bin width: " << bin_width << ", efficiency: " << efficiency << endl;
    }

    // FTF=false
    TFile *file_offline = new TFile(fullPath.c_str());
    TTree *tree_offline = dynamic_cast<TTree*>(file_offline->Get("physics"));
    for (int bin_num = min_bin_num; bin_num <= max_bin_num; bin_num *= 2) {
        tie(reco_z0, primary_vertexies, bin_width) = pv_reco(tree_offline, false, bin_num);

        int true_count = 0;
        int false_count = 0;

        for (size_t i = 0; i < reco_z0.size(); ++i) {
            if (reco_z0[i][0] < primary_vertexies[i] && primary_vertexies[i] < reco_z0[i][1]) {
                true_count++;
            } else {
                false_count++;
            }
        }

        double efficiency = static_cast<double>(true_count) / (true_count + false_count);
        g2->SetPoint(g2->GetN(), bin_width, efficiency);

        cout << "[offline] bin width: " << bin_width << ", efficiency: " << efficiency << endl;
    }

    TCanvas *c1 = new TCanvas("c1", "", 850, 700);
    g1->Draw("ALP");
    g2->Draw("LP same");

    TLegend *legend = new TLegend(0.6, 0.4, 0.8, 0.55);
    legend->AddEntry(g1, "FTF", "l");
    legend->AddEntry(g2, "Offline", "l");
    legend->SetBorderSize(0);
    legend->Draw();

    // ATLASラベルを追加
    TLatex latex;
    latex.SetNDC();
    latex.SetTextFont(72); // ATLASフォントスタイル
    latex.SetTextSize(0.04);
    latex.DrawLatex(0.5, 0.2, "ATLAS");

    latex.SetTextFont(42); // 標準のフォントスタイルに戻す
    latex.DrawLatex(0.62, 0.2, "Simulation Internal");

    c1->Print((string(basePath) + "/output/" + string(dir) + "/bin_width.pdf").c_str());
}
