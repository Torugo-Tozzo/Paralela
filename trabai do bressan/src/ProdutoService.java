import java.rmi.Remote;
import java.rmi.RemoteException;
import java.util.List;

public interface ProdutoService extends Remote {
    void adicionarProduto(Produto produto) throws RemoteException;
    List<Produto> listarProdutos() throws RemoteException;
    Produto buscarProduto(int id) throws RemoteException;
    void atualizarProduto(Produto produto) throws RemoteException;
    void deletarProduto(int id) throws RemoteException;
}
