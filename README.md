# NOMEN

Aplicação de análise forense digital post-mortem para sistemas Linux.

## Visão geral
O Nomen é uma prova de conceito de uma ferramenta gráfica, open source e extensível para a gestão de casos, fontes de dados e indexação de cópias forenses.

Inclui:
- gestão de casos;
- importação e organização de fontes de dados;
- indexação de imagens forenses;
- navegação em sistemas de ficheiros;
- execução assíncrona de ingestões e indexação.

## Objetivo
Integrar num único ambiente as principais operações de análise forense em Linux, com foco em desempenho, extensibilidade e usabilidade.

## Tecnologias
- Qt 6
- C++
- Firebird Embedded
- LMDB
- TSK

## Estrutura
- `src/` — Código da Aplicação
- `extensions/` — Módulos de Ingestão
- `tests/` — Testes Unitários
- `resources/` — Ícones

## TODO
- [ ] Novos módulos de ingestão para suportar mais tipos de análise e artefactos.
- [ ] Desenvolver extensões para visualizadores de conteúdo por tipo de ficheiro.
- [ ] Implementar jobs de pesquisa de conteúdo com filtragem e referência a ficheiros relevantes.
- [ ] Adicionar views para diferentes formas de apresentação e agregação da informação.
- [ ] Criar sistema de tags e grupos para classificação e organização de artefactos.
- [ ] Implementar criação de relatórios para apresentação dos resultados da investigação.
- [ ] Melhorar pequenas funcionalidades da interface que não foram priorizadas no projeto inicial.


