#!/bin/bash

# Test 1: Enviar pedido de execução válido várias vezes
echo "Test 1: Sending valid execution request multiple times"
for i in {1..10}; do
    ./bin/client execute 10 -u "ls -l"
done

# Test status: Enviar pedido de status
echo "Test status: Sending status request"
./bin/client status
