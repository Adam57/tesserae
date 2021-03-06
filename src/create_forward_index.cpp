/*
 * Copyright 2018 The Tesserae authors.
 *
 * For the full copyright and license information, please view the LICENSE file
 * that was distributed with this source code.
 */

#include "indri/CompressedCollection.hpp"
#include "indri/QueryEnvironment.hpp"
#include "indri/Repository.hpp"
#include <future>

#include "CLI/CLI.hpp"
#include "cereal/archives/binary.hpp"

#include "tesserae/field_map.hpp"
#include "tesserae/forward_index.hpp"

size_t url_slash_count(const std::string &url) {
    size_t            count       = 0;
    const std::string proto       = "://";
    const std::string param_delim = "?";
    size_t            pos         = url.find(proto);
    size_t            pos_q       = url.find(param_delim);

    if (pos_q < pos || std::string::npos == pos) {
        pos = 0;
    } else {
        pos += proto.size();
    }

    while (std::string::npos != (pos = url.find("/", pos + 1, 1))) {
        ++count;
    }

    return count;
}

static const std::vector<std::string> _fields = {"body", "title", "heading", "inlink", "a"};

int main(int argc, char const *argv[]) {
    std::string repo_path;
    std::string forward_index_file;

    CLI::App app{"Inverted index generator."};
    app.add_option("repo_path", repo_path, "Indri repo path")->required();
    app.add_option("forward_index_file", forward_index_file, "Forward index file")->required();
    CLI11_PARSE(app, argc, argv);

    std::ofstream               os(forward_index_file, std::ios::binary);
    cereal::BinaryOutputArchive archive(os);

    indri::collection::Repository repo;
    repo.openRead(repo_path);
    indri::collection::Repository::index_state state = repo.indexes();
    const auto &                               index = (*state)[0];

    FieldMap fields;
    fields.insert(*index, _fields);

    indri::api::QueryEnvironment indri_env;
    indri_env.addIndex(repo_path);

    {
        // dump size of vector
        size_t len = index->documentCount();
        archive(len);
        // pad document index zero (unused)
        Document zero;
        archive(zero);
    }

    size_t                              total_docs = index->documentCount();
    uint64_t                            docid      = index->documentBase();
    indri::index::TermListFileIterator *iter       = index->termListFileIterator();
    iter->startIteration();

    while (!iter->finished()) {
        std::vector<std::future<void>> futures;

        indri::index::TermList *list      = iter->currentEntry();
        auto &                  doc_terms = list->terms();
        Document                document;

        futures.push_back(std::async([&]() {
            auto url = indri_env.documentMetadata(std::vector<lemur::api::DOCID_T>{docid}, "url");
            document.set_url_stats({url_slash_count(url.at(0)), url.at(0).size()});
        }));

        std::set<uint32_t>    unique_terms_set(doc_terms.begin(), doc_terms.end());
        std::vector<uint32_t> unique_terms(unique_terms_set.begin(), unique_terms_set.end());
        document.set_unique_terms(unique_terms);

        futures.push_back(std::async([&]() {
            std::vector<uint32_t> terms(doc_terms.begin(), doc_terms.end());
            document.set_terms(terms);
            std::unordered_map<uint32_t, uint32_t> freqs;
            for (size_t i = 0; i < terms.size(); i++) {
                freqs[terms[i]] += 1;
            }

            for (auto &f : freqs) {
                document.set_freq(f.first, f.second);
            }
        }));

        std::unordered_map<uint16_t, std::vector<indri::index::FieldExtent>> field_list;
        for (auto &f : list->fields()) {
            if (fields.get().find(index->field(f.id)) != fields.get().end()) {
                document.set_tag_count(f.id, document.tag_count(f.id) + 1);
                field_list[f.id].push_back(f);
            }
        }

        std::vector<uint16_t> fv;
        for (auto &f : fields.get()) {
            fv.push_back(f.second);
        }
        document.set_fields(fv);

        std::unordered_map<size_t, std::unordered_map<uint32_t, uint32_t>> field_freqs;
        for (const auto &curr : field_list) {
            for (const auto &f : curr.second) {
                auto d_len = f.end - f.begin;
                document.set_field_len(f.id, document.field_len(f.id) + d_len);
                auto field_len_sqr = d_len * d_len;
                document.set_field_len_sum_sqrs(f.id,
                                                document.field_len_sum_sqrs(f.id) + field_len_sqr);

                if (document.field_max_len(f.id) < document.field_len(f.id)) {
                    document.set_field_max_len(f.id, document.field_len(f.id));
                }

                if (document.field_min_len(f.id) < document.field_len(f.id)) {
                    document.set_field_min_len(f.id, document.field_len(f.id));
                }

                for (size_t i = f.begin; i < f.end; ++i) {
                    field_freqs[f.id][doc_terms[i]] += 1;
                }
            }
        }
        for (auto &&freq : field_freqs) {
            for (auto &&f : freq.second) {
                document.set_freq(freq.first, f.first, f.second);
            }
        }

        for (auto &e : futures) {
            e.get();
        }

        document.compress();
        archive(document);

        if (docid % 10000 == 0 || docid == index->documentBase() || size_t(docid) == total_docs) {
            static size_t      last_len = 0;
            std::ostringstream oss;
            double             progress = double(docid) / double(total_docs);
            progress *= 100;
            std::cerr << std::string(last_len, '\b');
            oss.str("");
            oss << "documents processed: " << docid << " (" << int(progress) << "%)";
            last_len = oss.str().size();
            std::cerr << oss.str();
        }

        iter->nextEntry();
        ++docid;
    }
    delete iter;
    return 0;
}
