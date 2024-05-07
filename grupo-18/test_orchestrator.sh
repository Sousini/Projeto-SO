#!/bin/bash

# Teste 1: Enviar pedido de execução para o orchestrator com parâmetros inválidos
echo "Test 1: Sending execution request to orchestrator with invalid parameters"
./bin/orchestrator

# Teste 2: Enviar pedido de execução para o orchestrator com parâmetros válidos
echo "Test 2: Sending execution request to orchestrator with valid parameters"
./bin/orchestrator tmp 1
