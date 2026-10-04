#import "/template/src/lib.typ": *

#show: fiquill.with(
  title: [Symbolic execution of Sala programs],
  author: "Tomáš Krchňák",
  date: [December 2026],
  keywords: (
    "SALA",
    "symbolic execution",
    "program analysis",
    "software verification",
    "test generation",
    "path exploration",
    "symbolic execution tree",
    "path condition",
    "offline visualization",
    "Test-Comp",
  ),
)

#pages.title(thesis: [Master's Thesis])
#pages.declaration[
  Hereby I declare that this thesis is my original authorial work, which I have
  worked out on my own. All sources, references, and literature used or
  excerpted during elaboration of this work are properly cited and listed in
  complete reference to the due source.

  #parbreak()

  During the preparation of this thesis, I used ChatGPT
  (#link("https://chatgpt.com/")[https://chatgpt.com/]) for code-design
  discussions, faster drafting, and improving my writing style. I critically
  reviewed and verified its outputs and take full responsibility for the thesis
  and all submitted electronic attachments.
]
#pages.outline(target: heading-selector(levels: (1, 2, 3)))

#show: pages.start-content

#include "chapters/00_intro.typ"
#include "chapters/01_background.typ"
#include "chapters/02_scope.typ"
#include "chapters/03_design.typ"
#include "chapters/04_implementation.typ"
#include "chapters/05_methodology.typ"
#include "chapters/06_results.typ"
#include "chapters/07_limitations.typ"
#include "chapters/08_conclusion.typ"


#show: pages.start-appendices
#include "appendices/reproducibility.typ"
