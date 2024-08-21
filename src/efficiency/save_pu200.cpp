#include <iostream>
#include <fstream>
#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TCanvas.h>

using namespace std;

const char* basePath = getenv("WORKDIR");
const char* rootFilePath = getenv("REAL_DATA_DIR");
const char* mcRootFilePath_pu200_ttbar = getenv("MC_DATA_DIR_PU200_TTBAR");
const char* mcRootFilePath_pu200_zmumu = getenv("MC_DATA_DIR_PU200_Z_MUMU");
const char* mcRootFilePath_pu140_zmumu = getenv("MC_DATA_DIR_PU140_Z_MUMU");
const char* mcRootFilePath_pu0_zmumu = getenv("MC_DATA_DIR_PU0_Z_MUMU");
const char* dataDir = "src/efficiency/data";

tuple<vector<vector<double>>, vector<double>, double> pv_reco(TTree *tree, int bin_num) {
    vector<double> *id_trk_pt = nullptr;
    vector<float> *id_trk_z0 = nullptr;
    vector<int> *vxp_type = nullptr;
    vector<float> *vxp_z = nullptr;
    vector<vector<double>> reco_z0;
    vector<double> primary_vertexies;
    double bin_width;

    tree->SetBranchAddress("id_trk_pt", &id_trk_pt);
    tree->SetBranchAddress("id_trk_z0" ,&id_trk_z0);
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

    for (int entry = 0; entry < entries; entry++) {
        tree->GetEntry(entry);

        TH1D *tempHist = new TH1D("tempHist", "Temporary Histogram", bin_num, -300, 300);

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

void save_pu200() {
    vector<vector<double>> reco_z0;
    vector<double> primary_vertexies;
    double bin_width;

    string outputDir = string(basePath)+ "/" + string(dataDir) + "/";
    ofstream outFile_ttbar(outputDir + "ttbar_efficiency.txt");
    ofstream outFile_zmumu_200(outputDir + "zmumu_200_efficiency.txt");
    ofstream outFile_zmumu_140(outputDir + "zmumu_140_efficiency.txt");
    ofstream outFile_zmumu_0(outputDir + "zmumu_0_efficiency.txt");

    int max_bin_num = 65536;
    int min_bin_num = 256;

    // ttbar (PU200)
    string fullPath = string(basePath) + "/" + string(mcRootFilePath_pu200_ttbar);
    TFile *file = new TFile(fullPath.c_str());
    TTree *tree = dynamic_cast<TTree*>(file->Get("physics"));
    for (int bin_num = min_bin_num; bin_num <= max_bin_num; bin_num *= 2) {
        tie(reco_z0, primary_vertexies, bin_width) = pv_reco(tree, bin_num);

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
        outFile_ttbar << bin_width << " " << efficiency << endl;

        cout << "[ttbar] bin width: " << bin_width << ", efficiency: " << efficiency << endl;
    }

    // Zmumu (PU200)
    string fullPath_zmumu = string(basePath) + "/" + string(mcRootFilePath_pu200_zmumu);
    TFile *file_zmumu = new TFile(fullPath_zmumu.c_str());
    TTree *tree_zmumu = dynamic_cast<TTree*>(file_zmumu->Get("physics"));
    for (int bin_num = min_bin_num; bin_num <= max_bin_num; bin_num *= 2) {
        tie(reco_z0, primary_vertexies, bin_width) = pv_reco(tree_zmumu, bin_num);

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
        outFile_zmumu_200 << bin_width << " " << efficiency << endl;

        cout << "[Zmumu 200] bin width: " << bin_width << ", efficiency: " << efficiency << endl;
    }

    // Zmumu (PU140)
    string fullPath_zmumu_pu140 = string(basePath) + "/" + string(mcRootFilePath_pu140_zmumu);
    TFile *file_zmumu_pu140 = new TFile(fullPath_zmumu_pu140.c_str());
    TTree *tree_zmumu_pu140 = dynamic_cast<TTree*>(file_zmumu_pu140->Get("physics"));
    for (int bin_num = min_bin_num; bin_num <= max_bin_num; bin_num *= 2) {
        tie(reco_z0, primary_vertexies, bin_width) = pv_reco(tree_zmumu_pu140, bin_num);

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
        outFile_zmumu_140 << bin_width << " " << efficiency << endl;

        cout << "[Zmumu PU140] bin width: " << bin_width << ", efficiency: " << efficiency << endl;
    }

    // Zmumu (PU0)
    string fullPath_zmumu_pu0 = string(basePath) + "/" + string(mcRootFilePath_pu0_zmumu);
    TFile *file_zmumu_pu0 = new TFile(fullPath_zmumu_pu0.c_str());
    TTree *tree_zmumu_pu0 = dynamic_cast<TTree*>(file_zmumu_pu0->Get("physics"));
    for (int bin_num = min_bin_num; bin_num <= max_bin_num; bin_num *= 2) {
        tie(reco_z0, primary_vertexies, bin_width) = pv_reco(tree_zmumu_pu0, bin_num);

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
        outFile_zmumu_0 << bin_width << " " << efficiency << endl;

        cout << "[Zmumu PU0] bin width: " << bin_width << ", efficiency: " << efficiency << endl;
    }

    outFile_ttbar.close();
    outFile_zmumu_200.close();
    outFile_zmumu_140.close();
    outFile_zmumu_0.close();
}
