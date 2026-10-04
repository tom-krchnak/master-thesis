# salasym

Symbolic executor for SALA

[Read the latest compiled thesis PDF](https://tom-krchnak.github.io/master-thesis/salasym.pdf).

```shell
# optional compatibility patch for Boost >1.87
make patch

# configure, build, and install the compiler locally
# - run only once
make salac

# configure and build salasym (default: release)
# - run after every change
make salasym
# make PRESET=debug salasym

# compile the thesis PDF
make thesis

# continuously rebuild the thesis while editing
make watch

# compile C and run salasym
make verify FILE=path/to/program.c
# or run an already compiled SALA program
./impl/build/release/salasym/salasym path/to/program.json

# remove all build artifacts for fresh start
make clean
```


## Thesis plan

**Delivery assumption:** the thesis is due on 15.12. Planned implementation, experiments, reading, and first-draft writing end on 22.11. The three complete weeks from 23.11. through 13.12. are protected buffer. 14.12.–15.12. are final submission checks.

### Global checklist

- [ ] **Prove the baseline**
  - Produce a reproducible SALA run with classified results, required statistics, and—where Test-Comp is claimed—a generated test artifact validated by the selected harness.

- [ ] **Freeze a neutral task manifest**
  - Use LP64 tasks only. Record source, property, data model, inclusion/exclusion reason, expected result, command, and outcome; retain failures, unsupported tasks, timeouts, and crashes.

- [ ] **Cap the heuristic study**
  - Compare DFS, BFS, and at most one pilot-justified heuristic. Prefer coverage-directed search if coverage data exists; otherwise use seeded random state selection. Do not add merging, solver caching, or A* without pilot evidence.

- [ ] **Measure reproducibly**
  - Use BenchExec with matched limits and three repetitions per configuration. Record median, range, variation, wall/CPU time, MaxRSS, solver statistics, and validated tests or coverage.

- [ ] **State semantic limits**
  - Keep SALA input limitations and executor-model limitations explicit. A successful SALA run is not automatically a source-C correctness claim.

### 05.10. – 11.10. — scope, baseline, and thesis skeleton

- [ ] **Set the thesis scope**
  - Define the contribution as a correctness-oriented SALA symbolic executor with measured capabilities and limitations. Write the supported-property and excluded-feature matrix.

- [ ] **Stabilize the baseline**
  - Build Debug and Release, preserve a small semantic smoke suite, and document or reproduce each current limitation.

- [ ] **Define benchmark eligibility**
  - Draft the SV-COMP and Test-Comp manifest schema and predeclare LP64 inclusion criteria.

- [ ] **Create the thesis skeleton**
  - Create chapters for introduction, background, SALA/executor design, implementation, methodology, results, threats to validity, and conclusion.

- [ ] **Start the literature record**
  - Read and annotate the KLEE OSDI paper plus current SV-COMP and Test-Comp rules. Each note records claim, setup, metric, limitation, and destination chapter.

### 12.10. – 18.10. — benchmark-critical implementation

- [ ] **Finish the benchmark path**
  - Implement only capabilities needed by the declared subset: result classification, statistics, deterministic configuration capture, and Test-Comp test/model output.

- [ ] **Add semantic regression coverage**
  - Add and run Debug/Release regression cases for every supported semantic path used by the benchmark subset.

- [ ] **Draft design and implementation**
  - Write the design and implementation chapters while the decisions and code are current.

- [ ] **Read only relevant heuristic work**
  - Read the KLEE state-merging paper and one search-strategy source. Record why merging stays out of scope unless evidence changes the schedule.

### 19.10. – 25.10. — harness and pilot corpus

- [ ] **Build the measurement harness**
  - Create the BenchExec wrapper, compilation pipeline, result collector, and machine-readable run record.

- [ ] **Assemble pilot manifests**
  - Select small stratified SV-COMP and Test-Comp pilot sets covering declared properties and supported program shapes.

- [ ] **Run the DFS pilot**
  - Run every pilot task once and classify supported, unsupported, incorrect, timeout, crash, and harness failure separately.

- [ ] **Validate generated tests**
  - Validate tests before treating Test-Comp output as usable evidence.

- [ ] **Draft methodology and related work**
  - Write the evaluation protocol and related-work chapters from the reading notes.

### 26.10. – 01.11. — search experiment and feature freeze

- [ ] **Implement BFS**
  - Add BFS and verify that it preserves executor semantics and deterministic configuration capture.

- [ ] **Choose one third heuristic**
  - Implement one bounded heuristic only when the pilot exposes a measurable scheduling problem. Compare it with DFS and BFS on the pilot corpus.

- [ ] **Fix pilot blockers**
  - Fix correctness blockers found by the pilot; do not add unrelated semantics.

- [ ] **Freeze features on 01.11.**
  - After this date, allow only harness repair, correctness fixes, experiments, analysis, and writing.

### 02.11. – 08.11. — protocol freeze and Test-Comp campaign

- [ ] **Freeze the experiment protocol**
  - Freeze manifests, versions, BenchExec configuration, limits, seeds, and metric definitions on 08.11.

- [ ] **Run Test-Comp measurements**
  - Run all frozen search configurations three times. Rerun only failed infrastructure cells before continuing.

- [ ] **Write Test-Comp evidence**
  - Write methodology, validation, and limitation sections from collected data rather than hand-maintained notes.

### 09.11. – 15.11. — SV-COMP campaign

- [ ] **Run SV-COMP measurements**
  - Run the frozen configurations three times under the same protocol.

- [ ] **Retain every outcome**
  - Preserve unsupported tasks, timeouts, crashes, and incorrect verdicts alongside successful results.

- [ ] **Audit initial results**
  - Produce plots and tables; trace outliers to logs and task metadata.

- [ ] **Write results and threats**
  - Draft the results and threats-to-validity sections.

### 16.11. – 22.11. — analysis, full draft, and evidence freeze

- [ ] **Aggregate the evidence**
  - Complete configuration, benchmark-scope, and DFS/BFS/third-heuristic comparison tables.

- [ ] **Repair only measurement defects**
  - Resolve correctness or harness defects and rerun only affected cells under the frozen protocol.

- [ ] **Finish the full draft**
  - Complete the thesis, including limitations, negative results, and reproducibility appendix.

- [ ] **Freeze evidence on 22.11.**
  - Freeze the code revision, manifests, raw results, plotting scripts, paper notes, and draft.

### 23.11. – 13.12. — protected three-week buffer

- [ ] **Protect the buffer**
  - Do not add a new feature, heuristic, benchmark category, or literature thread.

- [ ] **Absorb overruns: 23.11. – 29.11.**
  - Repair implementation or harness failures and rerun affected measurements.

- [ ] **Incorporate feedback: 30.11. – 06.12.**
  - Repair argumentation, tables, figures, and reproducibility material.

- [ ] **Prepare submission: 07.12. – 13.12.**
  - Complete consistency, reference, figure/table, backup, and submission checks.

- [ ] **Cut scope in the prescribed order**
  - Drop the third heuristic first; then reduce the manifest only by predeclared eligibility criteria; then report the unavailable objective as a limitation. Do not spend buffer time on unmeasured optimization.

### 14.12. – 15.12. — final checks

- [ ] **Build the final artifact**
  - Build the thesis from a clean checkout.

- [ ] **Audit references and evidence**
  - Verify citation keys, bibliography, figures, tables, appendix references, and raw-result links.

- [ ] **Proofread the PDF**
  - Check typos, grammar, terminology, typesetting, page breaks, and PDF metadata.

- [ ] **Submit the reviewed artifact**
  - Read the final PDF end to end and submit that exact artifact.
