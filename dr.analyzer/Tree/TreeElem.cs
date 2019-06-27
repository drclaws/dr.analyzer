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
        private bool isModule;

        public TreeElem(string name, bool isModule, TreeNode parentNode)
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
            this.isModule = isModule;
            if (isModule)
            {
                this.treeViewNode.ImageIndex = 1;
                this.treeViewNode.SelectedImageIndex = 1;
            }
            else
            {
                this.treeViewNode.ImageIndex = 2;
                this.treeViewNode.SelectedImageIndex = 2;
            }
        }

        public void UpdateNode(System.Windows.Forms.TreeNode newNode)
        {
            this.treeViewNode = newNode;
            this.treeViewNode.Text += this.Name;
            if (this.isModule)
            {
                this.treeViewNode.ImageIndex = 1;
                this.treeViewNode.SelectedImageIndex = 1;
            }
            else
            {
                this.treeViewNode.ImageIndex = 2;
                this.treeViewNode.SelectedImageIndex = 2;
            }
        }

        public void MakeModule()
        {
            this.isModule = true;
            this.treeViewNode.ImageIndex = 1;
            this.treeViewNode.SelectedImageIndex = 1;
        }
    }
}
