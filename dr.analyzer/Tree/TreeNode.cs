using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DrAnalyzer.Tree
{
    class TreeNode
    {
        public string Name { get; private set; }
        private readonly TreeNode parentNode;
        private readonly List<TreeNode> childrenNodes = new List<TreeNode>();
        private readonly List<TreeElem> childrenElems = new List<TreeElem>();

        public bool ViewNodeOwned { get; private set; } = false;
        public System.Windows.Forms.TreeNode TreeViewNode { get; private set; }

        public TreeNode(Queue<string> pathElems, TreeNode parentNode = null)
        {
            this.parentNode = parentNode;
            this.Name = pathElems.Dequeue();

            if (parentNode != null && !parentNode.ViewNodeOwned)
            {
                if (parentNode.ViewNodeOwned)
                {
                    this.TreeViewNode = new System.Windows.Forms.TreeNode(this.Name + @"\");
                    parentNode.TreeViewNode.Nodes.Add(this.TreeViewNode);
                }
                else
                {
                    this.TreeViewNode = parentNode.TreeViewNode;
                    parentNode.TreeViewNode.Text += this.Name + @"\";
                }
            }
            else
            {
                this.TreeViewNode = new System.Windows.Forms.TreeNode(this.Name + @"\");
            }

            if (pathElems.Count == 1)
            {
                childrenElems.Add(new TreeElem(pathElems.Dequeue(), this));
            }
            else
            {
                childrenNodes.Add(new TreeNode(pathElems, this));
            }
        }

        public void AddFile(Queue<string> pathElems)
        {
            string name = pathElems.Dequeue();
            if (pathElems.Count == 0)
            {
                if (!this.ViewNodeOwned)
                {
                    this.MakeNodeOwned();
                }
                childrenElems.Add(new TreeElem(name, this));
            }
            else
            {
                bool found = false;
                foreach(TreeNode node in this.childrenNodes)
                {
                    if(node.Name == name)
                    {
                        node.AddFile(pathElems);
                        found = true;
                        break;
                    }
                }
                if(!found)
                {
                    if (!this.ViewNodeOwned)
                    {
                        this.MakeNodeOwned();
                    }
                    this.childrenNodes.Add(new TreeNode(pathElems, this));
                }
            }
        }

        private void MakeNodeOwned()
        {
            string ownedPath = this.GetOwnedPath();
            if (this.TreeViewNode.Parent != null)
            {
                System.Windows.Forms.TreeNode childViewNode = this.TreeViewNode,
                    parentViewNode = this.TreeViewNode.Parent;
                parentViewNode.Nodes.Remove(childViewNode);
                this.TreeViewNode = new System.Windows.Forms.TreeNode(ownedPath);
                parentViewNode.Nodes.Add(this.TreeViewNode);
                childViewNode.Text.Remove(0, ownedPath.Length + 1);
                this.TreeViewNode.Nodes.Add(childViewNode);
            }
            else
            {
                System.Windows.Forms.TreeNode newNode = (System.Windows.Forms.TreeNode)this.TreeViewNode.Clone();
                this.TreeViewNode.Nodes.Clear();
                this.TreeViewNode.Name = ownedPath;
                this.TreeViewNode.Nodes.Add(newNode);
                newNode.Name = "";
            }
            this.ViewNodeOwned = true;
        }

        public string GetOwnedPath()
        {
            if (this.parentNode == null || this.parentNode.ViewNodeOwned)
            {
                return this.Name;
            }
            else
            {
                return this.parentNode.GetOwnedPath() + @"\" + this.Name;
            }
        }
        
        public void UpdateNode(System.Windows.Forms.TreeNode newNode)
        {
            newNode.Name += this.Name + @"\";
            this.TreeViewNode = newNode;
            if (!this.ViewNodeOwned)
            {
                if (this.childrenNodes.Count != 0)
                {
                    this.childrenNodes[0].UpdateNode(newNode);
                }
                else
                {
                    this.childrenElems.First().UpdateNode(newNode);
                }
            }
        }

    }
}
