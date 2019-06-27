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

        public TreeNode(string name, Queue<string> pathElems, bool isModulePath, TreeNode parentNode)
        {
            this.Name = name;
            string nextName = pathElems.Dequeue();
            this.parentNode = parentNode;

            if (parentNode != null)
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

            if (pathElems.Count == 0)
            {
                childrenElems.Add(new TreeElem(nextName, isModulePath, this));
            }
            else
            {
                childrenNodes.Add(new TreeNode(nextName, pathElems, isModulePath, this));
            }
        }

        public TreeNode(string name, Queue<string> pathElems, bool isModulePath)
        {
            this.parentNode = null;
            this.Name = name;
            this.TreeViewNode = new System.Windows.Forms.TreeNode(name);
            this.ViewNodeOwned = true;

            string nextName = pathElems.Dequeue();
            if (pathElems.Count == 0)
            {
                childrenElems.Add(new TreeElem(nextName, isModulePath, this));
            }
            else
            {
                childrenNodes.Add(new TreeNode(nextName, pathElems, isModulePath, this));
            }

        }

        public void MakeModuleByPath(Queue<string> pathElems)
        {
            string nextElem = pathElems.Dequeue();

            if (pathElems.Count == 0)
            {
                int elemIndex = this.childrenElems.FindIndex(val => val.Name == nextElem);
                if (elemIndex != -1)
                {
                    this.childrenElems[elemIndex].MakeModule();
                }
            }
            else
            {
                int nodeIndex = this.childrenNodes.FindIndex(val => val.Name == nextElem);
                if (nodeIndex != -1)
                {
                    this.childrenNodes[nodeIndex].MakeModuleByPath(pathElems);
                }
            }
        }

        public void AddFile(Queue<string> pathElems, bool isModulePath)
        {
            string nextName = pathElems.Dequeue();
            if (pathElems.Count == 0)
            {
                if (!this.ViewNodeOwned)
                {
                    this.MakeNodeOwned();
                }
                childrenElems.Add(new TreeElem(nextName, isModulePath, this));
            }
            else
            {
                bool found = false;
                foreach(TreeNode node in this.childrenNodes)
                {
                    if(node.Name == nextName)
                    {
                        node.AddFile(pathElems, isModulePath);
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
                    this.childrenNodes.Add(new TreeNode(nextName, pathElems, isModulePath, this));
                }
            }
        }

        private void MakeNodeOwned()
        {
            string ownedPath = this.GetOwnedPath();

            System.Windows.Forms.TreeNode 
                childViewNode = this.TreeViewNode, 
                parentViewNode = this.TreeViewNode.Parent;

            parentViewNode.Nodes.Remove(childViewNode);
            this.TreeViewNode = new System.Windows.Forms.TreeNode(ownedPath);
            parentViewNode.Nodes.Add(this.TreeViewNode);
            childViewNode.Text = childViewNode.Text.Remove(0, ownedPath.Length + 1);
            this.TreeViewNode.Nodes.Add(childViewNode);

            this.parentNode.UpdateParentNode(this.TreeViewNode);

            /*
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
            }*/
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
        
        public void UpdateParentNode(System.Windows.Forms.TreeNode newNode)
        {
            if (!this.ViewNodeOwned)
            {
                this.TreeViewNode = newNode;
                this.parentNode.UpdateParentNode(newNode);
            }
        }
    }
}
