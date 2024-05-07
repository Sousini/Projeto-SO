#!/bin/bash

# Teste 1: Enviar pedido de execução sem argumentos suficientes
echo "Test 1: Sending execution request without enough arguments"
./bin/client execute

# Teste 2: Enviar pedido de execução com opção inválida
echo "Test 2: Sending execution request with invalid option"
./bin/client execute 10 -x "ls -l"

# Teste 3: Enviar pedido de execução válido
echo "Test 3: Sending valid execution request"
./bin/client execute 10 -u "ls -l"

# Teste 4: Enviar pedido de status
echo "Test 4: Sending status request"
./bin/client status