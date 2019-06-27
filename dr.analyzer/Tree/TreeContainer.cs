using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DrAnalyzer.Tree
{
    public class TreeContainer
    {
        private readonly List<TreeNode> childrenNodes = new List<TreeNode>();

        public System.Windows.Forms.TreeNode AddPath(string filePath, bool isModule)
        {
            Queue<string> FullPathQueue = TreeContainer.GetFullPathQueue(filePath);

            string rootName = FullPathQueue.Dequeue();
            int treeNodeIndex = childrenNodes.FindIndex(val => val.Name == rootName);
            
            if (treeNodeIndex == -1)
            {
                TreeNode newNode = new TreeNode(rootName, FullPathQueue, isModule);
                childrenNodes.Add(newNode);
                return newNode.TreeViewNode;
            }
            else
            {
                this.childrenNodes[treeNodeIndex].AddFile(FullPathQueue, isModule);
                return null;
            }
        }

        public void MakeModuleByPath(string filePath)
        {
            Queue<string> FullPathQueue = TreeContainer.GetFullPathQueue(filePath);
            string rootName = FullPathQueue.Dequeue();

            int treeNodeIndex = childrenNodes.FindIndex(val => val.Name == rootName);
            if (treeNodeIndex != -1)
            {
                this.childrenNodes[treeNodeIndex].MakeModuleByPath(FullPathQueue);
            }
        }

        private static Queue<string> GetFullPathQueue(string filePath)
        {
            List<string> FullPath = new List<string>();
            FullPath.Insert(0, System.IO.Path.GetFileName(filePath));

            System.IO.DirectoryInfo directory = System.IO.Directory.GetParent(filePath);
            while (directory != null)
            {
                FullPath.Insert(0, directory.Name);
                directory = System.IO.Directory.GetParent(directory.FullName);
            }
            return new Queue<string>(FullPath);
        }

        public void Clear()
        {
            this.childrenNodes.Clear();
        }
    }
}
