#### Caso de Uso: \<\<CRUD\>\> Gerenciar Fermentador

1. O usuário escolhe a operação:
  1. Variante "inserir".
  2. Variante "consultar".
  3. Variante "alterar".
  4. Variante "excluir".

##### Variante 1.1: Inserir
1.1.1 [IN] O usuário informa o ID do fermentador e sua capacidade.

##### Variante 1.2: Consultar
1.2.1 [OUT] O sistema apresenta uma lista de fermentadores.
1.2.2 [IN] O usuário seleciona um elemento da lista.
1.2.3 [OUT] O sistema apresenta as informações disponíveis do fermentador selecionado: sensor atribuído, capacidade máxima, volume atual, batelada e nível de fermentação atual.

##### Variante 1.3 Alterar
1.3.1 Inclui Variante 1.2.
1.3.2 [OUT] O sistema apresenta uma lista de sensores disponíveis.
1.3.3 [IN] O usuário seleciona um elemento da lista para alterar o sensor associado ao fermentador.
1.3.4 [IN] O usuário informa um novo valor para a capacidade do fermentador.

##### Variante 1.4 Excluir
1.4.1 [OUT] O sistema apresenta uma lista de fermentadores.
1.4.2 [IN] O usuário seleciona um fermentador da lista para excluir.

##### Exceção 1.1.1a ID do fermentador contém caracteres inválidos ou não corresponde ao formato esperado
1.1.1a.1 [OUT] O sistema informa que o formato do ID informado é inválido.
1.1.1a.2 Retorna ao passo 1

##### Exceção 1.1.1b ID do fermentador já existe no sistema
1.1.1b.1 [OUT] O sistema informa que o ID do fermentador já existe no sistema.
1.1.1b.2 Retorna ao passo 1

##### Exceção 1.1.1c e 1.3.4a Capacidade do fermentador é menor ou igual à 0 litros
1.1.1c.1 [OUT] O sistema informa que a capacidade do fermentador deve ser superior à 0 litros.
1.1.1c.2 Retorna ao passo 1

<!----------------------------------------------------------------------------------------------------------------------------------------------------------------------------->
----

#### Caso de Uso: \<\<CRUD\>\> Gerenciar SensorFermentacao

1. O usuário escolhe a operação:
  1. Variante "inserir".
  2. Variante "consultar".
  3. Variante "alterar".
  4. Variante "excluir".

##### Variante 1.1: Inserir
1.1.1 [IN] O usuário informa o ID do sensor.
? 1.1.2 [IN] O usuário informa o endereço IP do sensor.

##### Variante 1.2: Consultar
1.2.1 [OUT] O sistema apresenta uma lista de sensores.
1.2.2 [IN] O usuário seleciona um sensor da lista.
1.2.3 [OUT] O sistema apresenta as informações disponíveis do sensor disponível: ID, endereço IP, valor de medição atual e fermentador associado.

##### Variante 1.3 Alterar
1.3.1 Inclui Variante 1.2.
1.3.2 [OUT] O sistema apresenta uma lista de fermentadores disponíveis.
1.3.3 [IN] O usuário informa o novo fermentador ao qual o sensor deverá ser associado.

##### Variante 1.4 Excluir
1.4.1 [OUT] O sistema apresenta uma lista de sensores.
1.4.2 [IN] O usuário seleciona um sensor da lista para excluir.

##### Exceção 1.1.1a ID do sensor contém caracteres inválidos ou não corresponde ao formato esperado
1.1.1a.1 [OUT] O sistema informa que o formato do ID informado é inválido.
1.1.1a.2 Retorna ao passo 1

##### Exceção 1.1.1b ID do sensor já existe no sistema
1.1.1b.1 [OUT] O sistema informa que o ID do sensor já existe no sistema.
1.1.1b.2 Retorna ao passo 1

<!----------------------------------------------------------------------------------------------------------------------------------------------------------------------------->
----

#### Caso de Uso: \<\<REP\>\> Monitorar Fermentador

1. [OUT] O sistema apresenta uma lista de fermentadores disponíveis.
2. [IN] O usuário seleciona um fermentador da lista para monitorar.
3. [OUT] O sistema exibe um gráfico com a curva de fermentação da batelada presente no fermentador.

<!----------------------------------------------------------------------------------------------------------------------------------------------------------------------------->

#### Caso de Uso: Atribuir SensorFermentacao

1. [OUT] O sistema apresenta uma lista de sensores disponíveis.
2. [IN] O usuário seleciona um sensor da lista para atribuir o sensor ao fermentador.

<!----------------------------------------------------------------------------------------------------------------------------------------------------------------------------->

#### Caso de Uso: Associar Batelada-Fermentador

1. [OUT] O sistema apresenta uma lista de bateladas.
2. [IN] O usuário seleciona uma batelada da lista.
3. [OUT] O sistema apresenta uma lista de fermentadores disponíveis.
4. [IN] O usuário seleciona um fermentador para associar à batelada.

<!----------------------------------------------------------------------------------------------------------------------------------------------------------------------------->

#### Caso de Uso: Iniciar Nova Fermentacao

1. Inclui `Atribuir SensorFermentacao`
2. Associar `Batelada-Fermentador`

<!----------------------------------------------------------------------------------------------------------------------------------------------------------------------------->

#### Finalizar Fermentacao

1. [OUT] O sistema apresenta uma lista de fermentadores com conteúdo.
2. [IN] O usuário seleciona um dos fermentadores.
3. [OUT] O sistema apresenta as informações disponíveis do fermentador selecionado: sensor atribuído, capacidade máxima, volume atual, batelada e nível de fermentação atual.
4. [IN] O usuário seleciona a opção de finalizar a fermentação.