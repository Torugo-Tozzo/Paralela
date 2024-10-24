import java.io.*;
import java.net.Socket;
import java.util.List;
import java.util.Scanner;

public class Client {

    public static void main(String[] args) {
        Scanner scanner = new Scanner(System.in);
        boolean running = true;

        while (running) {
            System.out.println("===== Menu de Produtos =====");
            System.out.println("1. Adicionar Produto");
            System.out.println("2. Listar Produtos");
            System.out.println("3. Buscar Produto por ID");
            System.out.println("4. Atualizar Produto");
            System.out.println("5. Deletar Produto");
            System.out.println("6. Sair");
            System.out.print("Escolha uma opção: ");
            int opcao = scanner.nextInt();

            switch (opcao) {
                case 1:
                    realizarOperacao((output, input) -> {
                        System.out.print("Digite o ID do produto: ");
                        int id = scanner.nextInt();
                        scanner.nextLine();  // Limpar buffer

                        System.out.print("Digite o nome do produto: ");
                        String nome = scanner.nextLine();

                        System.out.print("Digite o preço do produto: ");
                        double preco = scanner.nextDouble();

                        Produto produto = new Produto(id, nome, preco);
                        output.writeUTF("ADD");
                        output.writeObject(produto);
                        output.flush();

                        System.out.println(input.readUTF());
                    });
                    break;

                case 2:
                    realizarOperacao((output, input) -> {
                        output.writeUTF("LIST");
                        output.flush();

                        List<Produto> produtos = (List<Produto>) input.readObject();
                        System.out.println("Lista de produtos: " + produtos);
                    });
                    break;

                case 3:
                    realizarOperacao((output, input) -> {
                        System.out.print("Digite o ID do produto que deseja buscar: ");
                        int idBusca = scanner.nextInt();

                        output.writeUTF("SEARCH");
                        output.writeInt(idBusca);
                        output.flush();

                        Produto produtoEncontrado = (Produto) input.readObject();
                        System.out.println("Produto encontrado: " + produtoEncontrado);
                    });
                    break;

                case 4:
                    realizarOperacao((output, input) -> {
                        System.out.print("Digite o ID do produto que deseja atualizar: ");
                        int idAtualizar = scanner.nextInt();
                        scanner.nextLine();  // Limpar buffer

                        System.out.print("Digite o novo nome do produto: ");
                        String novoNome = scanner.nextLine();

                        System.out.print("Digite o novo preço do produto: ");
                        double novoPreco = scanner.nextDouble();

                        Produto produtoParaAtualizar = new Produto(idAtualizar, novoNome, novoPreco);
                        output.writeUTF("UPDATE");
                        output.writeObject(produtoParaAtualizar);
                        output.flush();

                        System.out.println(input.readUTF());
                    });
                    break;

                case 5:
                    realizarOperacao((output, input) -> {
                        System.out.print("Digite o ID do produto que deseja deletar: ");
                        int idDeletar = scanner.nextInt();

                        output.writeUTF("DELETE");
                        output.writeInt(idDeletar);
                        output.flush();

                        System.out.println(input.readUTF());
                    });
                    break;

                case 6:
                    running = false;
                    break;

                default:
                    System.out.println("Opção inválida.");
                    break;
            }
        }
    }

    private static void realizarOperacao(Operacao operacao) {
        try (Socket socket = new Socket("localhost", 1099);
             ObjectOutputStream output = new ObjectOutputStream(socket.getOutputStream());
             ObjectInputStream input = new ObjectInputStream(socket.getInputStream())) {

            operacao.executar(output, input);

        } catch (IOException | ClassNotFoundException e) {
            e.printStackTrace();
        }
    }

    @FunctionalInterface
    interface Operacao {
        void executar(ObjectOutputStream output, ObjectInputStream input) throws IOException, ClassNotFoundException;
    }
}