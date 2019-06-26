using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DrAnalyzer.Tree
{
    class TreeElem
    {
        public readonly string Name;
        private readonly TreeNode parentNode;
        private System.Windows.Forms.TreeNode treeViewNode = null;

        public TreeElem(string name, TreeNode parentNode)
        {
            this.Name = name;
            this.parentNode = parentNode;
            if (this.parentNode.ViewNodeOwned)
            {
                this.treeViewNode = new System.Windows.Forms.TreeNode(name);
                this.parentNode.TreeViewNode.Nodes.Add(this.treeViewNode);
            }
            else
            {
                this.treeViewNode = this.parentNode.TreeViewNode;
                this.treeViewNode.Text += name;
            }
        }

        public void UpdateNode(System.Windows.Forms.TreeNode newNode)
        {
            this.treeViewNode = newNode;
            this.treeViewNode.Text += this.Name;
        }

    }
}
