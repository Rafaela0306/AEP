#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <locale.h>

#define MAX_USUARIOS 10000

// Estrutura para armazenar os dados do usu�rio
typedef struct {
    int id;
    char nome[100];
    char senha_hash[200]; // Armazena o hash com salt
} Usuario;

// Fun��o para gerar um salt aleat�rio
void gerarSalt(char *salt, size_t tamanho) {
    const char *caracteres = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    for (size_t i = 0; i < tamanho - 1; i++) {
        salt[i] = caracteres[rand() % strlen(caracteres)];
    }
    salt[tamanho - 1] = '\0';
}

// Fun��o de hash com salt
void gerarHashSenha(const char *senha, const char *salt, char *hash) {
    char senha_salt[200];
    snprintf(senha_salt, sizeof(senha_salt), "%s%s", senha, salt); // Concatena senha e salt

    unsigned int hash_value = 0;
    for (int i = 0; senha_salt[i] != '\0'; i++) {
        hash_value = (hash_value << 5) + senha_salt[i] + (hash_value >> 2);
    }
    sprintf(hash, "%s%08x", salt, hash_value); // Inclui o salt no in�cio do hash
}

// Fun��o para verificar se um ID j� existe
int idExiste(int id) {
    FILE *file = fopen("usuarios.txt", "r");
    if (file == NULL) return 0;

    Usuario usuario;
    while (fscanf(file, "%d;%99[^;];%199[^\n]\n", &usuario.id, usuario.nome, usuario.senha_hash) != EOF) {
        if (usuario.id == id) {
            fclose(file);
            return 1; // ID j� existe
        }
    }

    fclose(file);
    return 0; // ID n�o existe
}

// Fun��o para incluir um novo usu�rio
void incluirUsuario() {
    FILE *file = fopen("usuarios.txt", "a+");
    if (file == NULL) {
        perror("Erro ao abrir o arquivo");
        return;
    }

    Usuario usuario;
    char senha[100], salt[16], senha_hash[200];

    printf("Digite o ID do usu�rio: ");
    scanf("%d", &usuario.id);
    getchar(); // Limpa o buffer

    if (idExiste(usuario.id)) {
        printf("Erro: ID j� existe! Escolha outro.\n");
        fclose(file);
        return;
    }

    printf("Digite o nome do usu�rio: ");
    fgets(usuario.nome, sizeof(usuario.nome), stdin);
    usuario.nome[strcspn(usuario.nome, "\n")] = 0; // Remove a nova linha

    printf("Digite a senha do usu�rio: ");
    fgets(senha, sizeof(senha), stdin);
    senha[strcspn(senha, "\n")] = 0; // Remove a nova linha

    // Gera o salt e o hash da senha
    gerarSalt(salt, sizeof(salt));
    gerarHashSenha(senha, salt, senha_hash);

    // Escreve no arquivo
    fprintf(file, "%d;%s;%s\n", usuario.id, usuario.nome, senha_hash);
    fclose(file);
    printf("Usu�rio inclu�do com sucesso!\n");
}

// Fun��o para listar os usu�rios
void listarUsuarios() {
    FILE *file = fopen("usuarios.txt", "r");
    if (file == NULL) {
        printf("Nenhum usu�rio encontrado.\n");
        return;
    }

    Usuario usuario;
    printf("--- Lista de Usu�rios ---\n");
    while (fscanf(file, "%d;%99[^;];%199[^\n]\n", &usuario.id, usuario.nome, usuario.senha_hash) != EOF) {
        printf("ID: %d, Nome: %s, Senha (hash): %s\n", usuario.id, usuario.nome, usuario.senha_hash);
    }

    fclose(file);
}

// Fun��o para excluir um usu�rio
void excluirUsuario() {
    int id;
    printf("Digite o ID do usu�rio a ser exclu�do: ");
    scanf("%d", &id);
    getchar(); // Limpa o buffer

    FILE *file = fopen("usuarios.txt", "r");
    FILE *temp = fopen("temp.txt", "w");
    if (file == NULL || temp == NULL) {
        perror("Erro ao abrir o arquivo");
        return;
    }

    Usuario usuario;
    int encontrado = 0;

    while (fscanf(file, "%d;%99[^;];%199[^\n]\n", &usuario.id, usuario.nome, usuario.senha_hash) != EOF) {
        if (usuario.id == id) {
            encontrado = 1; // Usu�rio encontrado, n�o copia para o novo arquivo
        } else {
            fprintf(temp, "%d;%s;%s\n", usuario.id, usuario.nome, usuario.senha_hash);
        }
    }

    fclose(file);
    fclose(temp);

    remove("usuarios.txt");
    rename("temp.txt", "usuarios.txt");

    if (encontrado) {
        printf("Usu�rio exclu�do com sucesso!\n");
    } else {
        printf("Usu�rio com ID %d n�o encontrado.\n", id);
    }
}

// Fun��o para alterar um usu�rio
void alterarUsuario() {
    int id;
    printf("Digite o ID do usu�rio a ser alterado: ");
    scanf("%d", &id);
    getchar(); // Limpa o buffer

    FILE *file = fopen("usuarios.txt", "r");
    FILE *temp = fopen("temp.txt", "w");
    if (file == NULL || temp == NULL) {
        perror("Erro ao abrir o arquivo");
        return;
    }

    Usuario usuario;
    char senha[100], salt[16], senha_hash[200];
    int encontrado = 0;

    while (fscanf(file, "%d;%99[^;];%199[^\n]\n", &usuario.id, usuario.nome, usuario.senha_hash) != EOF) {
        if (usuario.id == id) {
            encontrado = 1;

            printf("Digite o novo nome: ");
            fgets(usuario.nome, sizeof(usuario.nome), stdin);
            usuario.nome[strcspn(usuario.nome, "\n")] = 0;

            printf("Digite a nova senha: ");
            fgets(senha, sizeof(senha), stdin);
            senha[strcspn(senha, "\n")] = 0;

            // Gera o novo hash da senha
            gerarSalt(salt, sizeof(salt));
            gerarHashSenha(senha, salt, senha_hash);

            fprintf(temp, "%d;%s;%s\n", usuario.id, usuario.nome, senha_hash);
        } else {
            fprintf(temp, "%d;%s;%s\n", usuario.id, usuario.nome, usuario.senha_hash);
        }
    }

    fclose(file);
    fclose(temp);

    remove("usuarios.txt");
    rename("temp.txt", "usuarios.txt");

    if (encontrado) {
        printf("Usu�rio alterado com sucesso!\n");
    } else {
        printf("Usu�rio com ID %d n�o encontrado.\n", id);
    }
}

// Fun��o principal
int main() {
    setlocale(LC_ALL, "");
    srand(time(NULL)); // Inicializa o gerador de n�meros aleat�rios
    int opcao;

    do {
        printf("\n--- Menu ---\n");
        printf("1. Incluir Usu�rio\n");
        printf("2. Alterar Usu�rio\n");
        printf("3. Excluir Usu�rio\n");
        printf("4. Listar Usu�rios\n");
        printf("0. Sair\n");
        printf("Escolha uma op��o: ");
        scanf("%d", &opcao);
        getchar(); // Limpa o buffer

        switch (opcao) {
            case 1:
                incluirUsuario();
                break;
            case 2:
                alterarUsuario();
                break;
            case 3:
                excluirUsuario();
                break;
            case 4:
                listarUsuarios();
                break;
            case 0:
                printf("Saindo...\n");
                break;
            default:
                printf("Op��o inv�lida!\n");
        }
    } while (opcao != 0);

    return 0;
}
