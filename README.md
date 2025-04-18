atualmente o server fica escutando por mensagens e retorna "hello from server" pra qualquer ip que ele tiver recebido mensagem

quando tu executa o cliente a primeira coisa que ele faz é mandar um broadcast e esperar de volta uma mensagem de "hello from server", se ele receber ele vai dizer "connected" e salvar o ip/porta do server

depois disso o cliente recebe da entrada padrao inteiros que ele manda pro server

o server printa qualquer inteiro que receber na saida padrao
