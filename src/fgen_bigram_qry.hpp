#pragma once

#include <unordered_map>
#include <string>

#include "fgen_term_qry.hpp"


typedef struct {
    std::string   bigram;
    uint64_t cf;
    uint64_t cdf;
    double   geo_mean;
    double   bm25_median_score;
    double   bm25_firstq_score;
    double   bm25_thirdq_score;
    double   bm25_max_score;
    double   bm25_min_score;
    double   bm25_mean_score;
    double   bm25_score_variance;
    double   bm25_score_stddev;
    double   bm25_score_confidence;
    double   bm25_score_harmonic_mean;
    double   tf_median_score;
    double   tf_firstq_score;
    double   tf_thirdq_score;
    double   tf_max_score;
    double   tf_min_score;
    double   tf_mean_score;
    double   tf_score_variance;
    double   tf_score_stddev;
    double   tf_score_confidence;
    double   tf_score_harmonic_mean;
    double   lm_median_score;
    double   lm_firstq_score;
    double   lm_thirdq_score;
    double   lm_max_score;
    double   lm_min_score;
    double   lm_mean_score;
    double   lm_score_variance;
    double   lm_score_stddev;
    double   lm_score_confidence;
    double   lm_score_harmonic_mean;
    double   dfr_median_score;
    double   dfr_firstq_score;
    double   dfr_thirdq_score;
    double   dfr_max_score;
    double   dfr_min_score;
    double   dfr_mean_score;
    double   dfr_score_variance;
    double   dfr_score_stddev;
    double   dfr_score_confidence;
    double   dfr_score_harmonic_mean;
    double   be_median_score;
    double   be_firstq_score;
    double   be_thirdq_score;
    double   be_max_score;
    double   be_min_score;
    double   be_mean_score;
    double   be_score_variance;
    double   be_score_stddev;
    double   be_score_confidence;
    double   be_score_harmonic_mean;
    double   dph_median_score;
    double   dph_firstq_score;
    double   dph_thirdq_score;
    double   dph_max_score;
    double   dph_min_score;
    double   dph_mean_score;
    double   dph_score_variance;
    double   dph_score_stddev;
    double   dph_score_confidence;
    double   dph_score_harmonic_mean;
    double   pr_median_score;
    double   pr_firstq_score;
    double   pr_thirdq_score;
    double   pr_max_score;
    double   pr_min_score;
    double   pr_mean_score;
    double   pr_score_variance;
    double   pr_score_stddev;
    double   pr_score_confidence;
    double   pr_score_harmonic_mean;
} bigram_t;


std::unordered_map<std::string, bigram_t> load_bigrammap(const char *fname);
std::string         fgen_bigram_qry_main(std::unordered_map<std::string, bigram_t> &bigrammap, int qnum, char **termv, size_t termc);
