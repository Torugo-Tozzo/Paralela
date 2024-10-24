import java.rmi.RemoteException;
import java.rmi.server.UnicastRemoteObject;
import java.util.ArrayList;
import java.util.List;

public class ProdutoServiceImpl extends UnicastRemoteObject implements ProdutoService {
    private List<Produto> produtos;

    public ProdutoServiceImpl() throws RemoteException {
        produtos = new ArrayList<>();
    }

    @Override
    public void adicionarProduto(Produto produto) throws RemoteException {
        produtos.add(produto);
        System.out.println("Produto adicionado: " + produto);
    }

    @Override
    public List<Produto> listarProdutos() throws RemoteException {
        return produtos;
    }

    @Override
    public Produto buscarProduto(int id) throws RemoteException {
        return produtos.stream().filter(p -> p.getId() == id).findFirst().orElse(null);
    }

    @Override
    public void atualizarProduto(Produto produto) throws RemoteException {
        Produto existente = buscarProduto(produto.getId());
        if (existente != null) {
            existente.setNome(produto.getNome());
            existente.setPreco(produto.getPreco());
            System.out.println("Produto atualizado: " + produto);
        }
    }

    @Override
    public void deletarProduto(int id) throws RemoteException {
        produtos.removeIf(p -> p.getId() == id);
        System.out.println("Produto deletado: ID " + id);
    }
}
