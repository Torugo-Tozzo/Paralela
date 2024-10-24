import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.ArrayList;
import java.util.List;

public class Server {

    private static List<Produto> produtos = new ArrayList<>();

    public static void main(String[] args) {
        try (ServerSocket serverSocket = new ServerSocket(1099)) {
            System.out.println("Servidor pronto para CRUD de produtos via Socket.");

            while (true) {
                Socket socket = serverSocket.accept();
                new Thread(new ClientHandler(socket)).start(); // Trata múltiplos clientes
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private static class ClientHandler implements Runnable {
        private final Socket socket;

        public ClientHandler(Socket socket) {
            this.socket = socket;
        }

        @Override
        public void run() {
            try (ObjectInputStream input = new ObjectInputStream(socket.getInputStream());
                 ObjectOutputStream output = new ObjectOutputStream(socket.getOutputStream())) {

                String command = input.readUTF();
                switch (command) {
                    case "ADD":
                        Produto produto = (Produto) input.readObject();
                        produtos.add(produto);
                        output.writeUTF("Produto adicionado com sucesso!");
                        output.flush();
                        break;

                    case "LIST":
                        output.writeObject(produtos);
                        output.flush();
                        break;

                    case "SEARCH":
                        int idBusca = input.readInt();
                        Produto produtoEncontrado = produtos.stream()
                                .filter(p -> p.getId() == idBusca)
                                .findFirst()
                                .orElse(null);
                        output.writeObject(produtoEncontrado);
                        output.flush();
                        break;

                    case "UPDATE":
                        Produto produtoParaAtualizar = (Produto) input.readObject();
                        for (int i = 0; i < produtos.size(); i++) {
                            if (produtos.get(i).getId() == produtoParaAtualizar.getId()) {
                                produtos.set(i, produtoParaAtualizar);
                                output.writeUTF("Produto atualizado com sucesso!");
                                output.flush();
                                break;
                            }
                        }
                        break;

                    case "DELETE":
                        int idDeletar = input.readInt();
                        produtos.removeIf(p -> p.getId() == idDeletar);
                        output.writeUTF("Produto deletado com sucesso!");
                        output.flush();
                        break;

                    default:
                        output.writeUTF("Comando inválido!");
                        output.flush();
                        break;
                }
            } catch (IOException | ClassNotFoundException e) {
                e.printStackTrace();
            }
        }
    }
}
